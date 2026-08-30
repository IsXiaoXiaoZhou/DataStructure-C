<!-- docs/.vitepress/theme/visualizer/renderers/Tree.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

/**
 * Tree 渲染器 —— 统一树形态（二叉是多叉特例，另覆盖 B 树 / 森林 / 并查集森林）。
 *
 * Step.state 契约：
 * {
 *   nodes: {
 *     id: number                      // 结点唯一 id（画在哪由布局算法决定，与 id 大小无关）
 *     keys: (number|string)[]         // 键集：0 个=外部/虚拟结点（虚线框，如森林总根），1 个=二叉，
 *                                     //   多个=B 树结点（孩子数约定 = 键数+1）
 *     children: number[]              // 孩子 id 数组（左→右序）
 *     sides?: ('L'|'R')[]             // 孩子 L/R 槽位（与 children 平行，仅二叉结点需要）：
 *                                     //   单侧孩子按槽位渲染；缺省按一般树均匀铺开
 *     color?: 'R'|'B'                 // 红黑树填充：红底 / 黑底白字（缺省中性底）
 *     nodeLabel?: string              // 结点上方小标签（权值/编号/BF/线索说明）
 *     edgeLabel?: string              // 连向双亲的边中点小标签（如 2i / 2i+1）
 *     keyHighlight?: number[]         // keys 下标：绿色（已完成/线索/命中）
 *     keyActive?: number[]            // keys 下标：橙色（当前比较/操作），优先于 highlight
 *     edgeActive?: number[]           // children 下标：本结点第 i 条孩子边高亮
 *   }[]
 *   rootId: number|null               // 根 id；null（或 nodes 空）画"空"框
 *   note?: string                     // 顶部注释行（结论/叶链/性质）
 * }
 *
 * 布局契约（SSR 安全，纯 computed）：
 *   按深度分行；子树宽度优先——叶宽=结点盒宽，内结点宽=max(盒宽, Σ孩子子树宽+间距)，
 *   父结点画在子树宽度中线。连线挂点：
 *     - 孩子数 = 键数+1 且键数 ≥ 2（B 树）：挂在键格边界，盒下缘画 10px 短横；
 *     - 键 1 个孩子 2 个（二叉）：挂盒宽 3/10 与 7/10 处；
 *     - 其余（一般树/单孩）：在盒宽上均匀铺开。
 * 高亮契约：keyActive（橙）> keyHighlight（绿）；edgeActive[i] 高亮第 i 条孩子边与 edgeLabel。
 */
interface TNode {
  id: number
  keys: (number | string)[]
  children: number[]
  sides?: ('L' | 'R')[]
  color?: 'R' | 'B'
  nodeLabel?: string
  edgeLabel?: string
  keyHighlight?: number[]
  keyActive?: number[]
  edgeActive?: number[]
}
interface TreeState { nodes: TNode[]; rootId: number | null; note?: string }

const props = defineProps<{ step: Step | null }>()

const view = computed<TreeState | null>(() => {
  if (!props.step) return null
  const o = props.step.state as TreeState
  if (o && Array.isArray(o.nodes)) return o
  return null
})

// ---------- 几何常量 ----------
const KEY_W = 32          // 单键格宽
const NODE_H = 38         // 结点盒高
const H_GAP = 34          // 兄弟子树间距
const LEVEL_H = 92        // 层距
const TOP = 52            // 顶部留白（nodeLabel 空）
const LEFT = 18           // 左右留白
const DROP = 16           // 连线出盒后的竖直段

const nodeW = (n: TNode) => (n.keys.length === 0 ? 46 : Math.max(46, n.keys.length * KEY_W + 14))

const byId = computed(() => {
  const m = new Map<number, TNode>()
  for (const n of view.value?.nodes ?? []) m.set(n.id, n)
  return m
})

/** 二叉结点的孩子槽位展开：L/R 两槽，缺位为 null（占零宽但仍留间距，使单侧孩子偏在一侧） */
const slotsOf = (n: TNode): (number | null)[] => {
  if (n.keys.length <= 1 && n.sides && n.sides.length === n.children.length) {
    const slots: (number | null)[] = [null, null]
    n.children.forEach((c, i) => { slots[n.sides![i] === 'R' ? 1 : 0] = c })
    return slots
  }
  return [...n.children]
}

interface Geo { x: Record<number, number>; depth: Record<number, number>; width: number; height: number; maxDepth: number }

/** 子树宽度优先布局：返回每结点左上角 x 与深度 y（x 已含 LEFT 边距） */
const geo = computed<Geo>(() => {
  const g: Geo = { x: {}, depth: {}, width: 240, height: 140, maxDepth: 0 }
  const st = view.value
  const root = st?.rootId
  if (st == null || root == null || !byId.value.has(root)) return g

  const widthOf = (id: number, seen: Set<number>): number => {
    const n = byId.value.get(id)
    if (!n || seen.has(id)) return nodeW(n ?? ({ keys: [], children: [] } as TNode))
    seen.add(id)
    const slots = slotsOf(n).map(c => (c != null && !seen.has(c) ? c : null))
    if (!slots.some(c => c != null)) return nodeW(n)
    let sum = 0
    for (const c of slots) sum += (c != null ? widthOf(c, seen) : 0)
    return Math.max(nodeW(n), sum + H_GAP * (slots.length - 1))
  }

  const total = widthOf(root, new Set())
  const place = (id: number, depth: number, left: number, seen: Set<number>): number => {
    const n = byId.value.get(id)
    if (!n || seen.has(id) || g.x[id] !== undefined) return left
    seen.add(id)
    const w = widthOf(id, new Set())
    g.x[id] = left + (w - nodeW(n)) / 2 + LEFT   // 父结点画在子树宽度中线
    g.depth[id] = depth
    g.maxDepth = Math.max(g.maxDepth, depth)
    let cur = left
    for (const c of slotsOf(n)) {
      if (c == null) { cur += H_GAP; continue }  // 缺位槽：零宽占位，推进间距
      if (!byId.value.has(c) || g.x[c] !== undefined) { cur += widthOf(c, new Set()) + H_GAP; continue }
      const cw = widthOf(c, new Set())
      place(c, depth + 1, cur, seen)
      cur += cw + H_GAP                          // 子树依次铺开，间距 H_GAP
    }
    return left + w
  }
  place(root, 0, 0, new Set())
  g.width = total + LEFT * 2
  g.height = TOP + (g.maxDepth + 1) * LEVEL_H + 34
  return g
})

const yOf = (id: number) => TOP + (geo.value.depth[id] ?? 0) * LEVEL_H

/** 连线挂点 x（相对结点盒左缘的绝对坐标） */
function attachX(n: TNode, x: number, i: number): number {
  const m = n.children.length
  const k = n.keys.length
  const w = nodeW(n)
  if (k <= 1 && n.sides && n.sides.length === m) {
    // 二叉带槽位：L 挂 3/10、R 挂 7/10（单侧孩子也挂对侧位）
    return x + (n.sides[i] === 'R' ? w * 0.7 : w * 0.3)
  }
  if (m === k + 1 && k >= 2) {
    // B 树形态：第 0 条挂最左、最后一条挂最右，中间挂键格边界
    const pitch = (w - 14) / k
    if (i === 0) return x + 7
    if (i === m - 1) return x + w - 7
    return x + 7 + i * pitch
  }
  if (k === 1 && m === 2) return x + (i === 0 ? w * 0.3 : w * 0.7)
  return x + (w * (i + 1)) / (m + 1)
}

interface EdgeView { key: string; from: number; to: number; ax: number; ay: number; bx: number; by: number; hot: boolean; label?: string; tick: boolean }

const edges = computed<EdgeView[]>(() => {
  const st = view.value
  if (!st) return []
  const out: EdgeView[] = []
  for (const n of st.nodes) {
    if (geo.value.x[n.id] === undefined) continue
    const x = geo.value.x[n.id]
    const y = yOf(n.id)
    n.children.forEach((cid, i) => {
      if (geo.value.x[cid] === undefined) return
      const ax = attachX(n, x, i)
      const ay = y + NODE_H
      const bx = geo.value.x[cid] + nodeW(byId.value.get(cid)!) / 2
      const by = yOf(cid) - 2
      out.push({
        key: `${n.id}-${cid}`,
        from: n.id, to: cid,
        ax, ay, bx, by,
        hot: (n.edgeActive ?? []).includes(i),
        label: byId.value.get(cid)?.edgeLabel,
        tick: n.keys.length >= 2 && n.children.length === n.keys.length + 1
      })
    })
  }
  return out
})

const keyCls = (n: TNode, j: number) => ({
  key: true,
  kactive: (n.keyActive ?? []).includes(j),
  kdone: (n.keyHighlight ?? []).includes(j) && !(n.keyActive ?? []).includes(j)
})
const keyText = (v: number | string) => (v === '' ? '·' : String(v))
const isDark = (n: TNode) => n.color === 'B'
</script>

<template>
  <div v-if="view" class="tree">
    <div v-if="view.note" class="note">{{ view.note }}</div>

    <!-- 空树 -->
    <svg v-if="view.rootId === null || !byId.has(view.rootId)" class="pic" viewBox="0 0 240 120" role="img">
      <rect class="emptybox" x="82" y="38" width="76" height="40" rx="8" />
      <text class="emptytxt" x="120" y="63">空</text>
    </svg>

    <svg v-else class="pic" :viewBox="`0 0 ${Math.max(240, geo.width)} ${geo.height}`"
         :style="{ maxWidth: Math.min(860, Math.max(240, geo.width) * 1.25) + 'px' }" role="img">
      <!-- 连线在下、结点在上 -->
      <g v-for="e in edges" :key="e.key">
        <path class="edge" :class="{ hot: e.hot }"
              :d="`M ${e.ax} ${e.ay} L ${e.ax} ${e.ay + DROP} C ${e.ax} ${e.ay + DROP + 14}, ${e.bx} ${e.by - 20}, ${e.bx} ${e.by}`" />
        <line v-if="e.tick" class="tick" :class="{ hot: e.hot }"
              :x1="e.ax - 5" :y1="e.ay + DROP" :x2="e.ax + 5" :y2="e.ay + DROP" />
        <text v-if="e.label" class="elabel" :class="{ hot: e.hot }"
              :x="(e.ax + e.bx) / 2" :y="(e.ay + DROP + e.by) / 2 + 3">{{ e.label }}</text>
      </g>

      <g v-for="n in view.nodes" :key="n.id"
         :transform="`translate(${geo.x[n.id] ?? 0}, ${yOf(n.id)})`"
         :class="['node', n.color === 'R' ? 'red' : '', n.color === 'B' ? 'black' : '']">
        <text v-if="n.nodeLabel" :x="nodeW(n) / 2" :y="-8" class="nlabel">{{ n.nodeLabel }}</text>
        <rect class="frame" :width="nodeW(n)" :height="NODE_H" rx="7" :class="{ phantom: n.keys.length === 0 }" />
        <g v-for="(k, j) in n.keys" :key="j" :class="keyCls(n, j)">
          <rect class="keyrect" :x="7 + j * ((nodeW(n) - 14) / n.keys.length)" y="4"
                :width="(nodeW(n) - 14) / n.keys.length - 3" :height="NODE_H - 8" rx="4" />
          <text class="keytxt" :x="7 + j * ((nodeW(n) - 14) / n.keys.length) + ((nodeW(n) - 14) / n.keys.length - 3) / 2"
                :y="NODE_H / 2 + 5">{{ keyText(k) }}</text>
        </g>
      </g>
    </svg>

    <div class="legend">
      <span class="chip"><i class="swatch kactive"></i>当前键位</span>
      <span class="chip"><i class="swatch kdone"></i>已定/命中</span>
      <span class="chip"><i class="line hot"></i>操作边</span>
      <span class="chip"><i class="swatch red"></i>红结点</span>
      <span class="chip"><i class="swatch black"></i>黑结点</span>
      <span class="chip"><i class="swatch phantom"></i>外部/虚拟结点</span>
    </div>
  </div>
</template>

<style scoped>
.tree { padding: 10px 0 4px; }
.note { font-size: 13px; margin-bottom: 6px; padding: 6px 10px; border-radius: 6px; background: var(--vp-c-bg-soft); display: inline-block; }
.pic { width: 100%; display: block; margin: 0 auto; }

.node .frame { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); stroke-width: 1.4; transition: fill .25s, stroke .25s; }
.node .frame.phantom { fill: transparent; stroke-dasharray: 4 4; stroke-width: 1.2; }
.node.red .frame { fill: var(--vp-c-red-3); stroke: var(--vp-c-red-1); }
.node.black .frame { fill: var(--vp-c-black); stroke: var(--vp-c-black); }

.keyrect { fill: transparent; }
.key.kactive .keyrect { fill: var(--vp-c-yellow-2); stroke: var(--vp-c-yellow-1); stroke-width: 1.2; }
.key.kdone .keyrect { fill: var(--vp-c-green-2); stroke: var(--vp-c-green-2); }
.keytxt { font-size: 14px; font-weight: 600; text-anchor: middle; fill: var(--vp-c-text-1); font-variant-numeric: tabular-nums; }
.node.black .keytxt { fill: #fff; }
.node.red .keytxt { fill: var(--vp-c-text-1); }
.key.kactive .keytxt, .key.kdone .keytxt { fill: var(--vp-c-text-1); }
.node.black .key.kactive .keytxt, .node.black .key.kdone .keytxt { fill: var(--vp-c-text-1); }

.nlabel { font-size: 11.5px; text-anchor: middle; fill: var(--vp-c-text-2); }

.edge { fill: none; stroke: var(--vp-c-text-2); stroke-width: 1.5; }
.edge.hot { stroke: var(--vp-c-brand-1); stroke-width: 2.2; }
.tick { stroke: var(--vp-c-text-2); stroke-width: 1.5; }
.tick.hot { stroke: var(--vp-c-brand-1); stroke-width: 2.2; }
.elabel { font-size: 10px; text-anchor: middle; fill: var(--vp-c-text-3); }
.elabel.hot { fill: var(--vp-c-brand-1); font-weight: 600; }

.emptybox { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); stroke-dasharray: 4 4; }
.emptytxt { font-size: 14px; text-anchor: middle; fill: var(--vp-c-text-2); }

.legend { display: flex; gap: 12px; margin-top: 8px; font-size: 12px; opacity: .85; flex-wrap: wrap; align-items: center; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.kactive { background: var(--vp-c-yellow-2); border-color: var(--vp-c-yellow-1); }
.swatch.kdone { background: var(--vp-c-green-2); border-color: var(--vp-c-green-2); }
.swatch.red { background: var(--vp-c-red-3); border-color: var(--vp-c-red-1); }
.swatch.black { background: var(--vp-c-black); }
.swatch.phantom { background: transparent; border-style: dashed; }
.line { display: inline-block; width: 22px; height: 0; vertical-align: 4px; }
.line.hot { border-top: 2px solid var(--vp-c-brand-1); }
</style>
