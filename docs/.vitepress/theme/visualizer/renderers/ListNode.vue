<!-- docs/.vitepress/theme/visualizer/renderers/ListNode.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

/**
 * ListNode 渲染器 —— 链表结点 + 箭头（SVG 横排）。
 *
 * Step.state 契约：
 * {
 *   nodes: {
 *     id: number                    // 结点唯一 id；渲染位置按数组顺序横排（与 id 大小无关）
 *     value: number|string          // 数据域；head 结点显示"头结点"
 *     next?: number|null            // 后继结点 id；null/缺省画"打地"横线（NULL）
 *     prev?: number|null            // 前驱结点 id；有值才画三格双向结点 + 虚线回指箭头
 *     head?: boolean                // 头结点（数据格显示"头结点"）
 *     tail?: boolean                // 尾结点标记（生成器侧语义标记，供测试断言，渲染器不单独画）
 *     ring?: boolean                // 该结点的 next 画大弧线回指目标结点（循环链表/跨接）
 *     highlight?: 'none'|'active'|'done'   // 结点三态：当前操作 / 已完成
 *     label?: string                // 结点上方小标签（如"prev""新结点""front=rear"）
 *   }[]
 *   headId: number|null             // 头指针指向的结点 id；null 表示空链（外指针打地）
 *   note?: string                   // 顶部注释行（状态码/结论）
 *   headLabel?: string              // 外部头指针名，缺省 "head"（链栈传 "top"）
 * }
 *
 * 高亮契约：
 *   - 主通道：node.highlight（'active' 黄 / 'done' 绿），逐帧由生成器重建；
 *   - 叠加通道：Step.active / Step.highlights 的数字按「nodes 数组下标」解释
 *     （不是 id！生成器需自行 findIndex 映射），active 优先于 done；
 *   - 箭头：源或目标结点为 active 时，next/prev 箭头同步高亮。
 */
interface LNNode {
  id: number
  value: number | string
  next?: number | null
  prev?: number | null
  head?: boolean
  tail?: boolean
  ring?: boolean
  highlight?: 'none' | 'active' | 'done'
  label?: string
}
interface LNState {
  nodes: LNNode[]
  headId: number | null
  note?: string
  headLabel?: string
}

const props = defineProps<{ step: Step | null }>()

const CELL = 46, NODE_H = 48, GAP = 54, TOP = 62

const view = computed<LNState | null>(() => {
  if (!props.step) return null
  const o = props.step.state as LNState
  if (o && Array.isArray(o.nodes)) return o
  return null
})

const nodeW = (n: LNNode) => (n.prev != null ? CELL * 3 : CELL * 2)

/** 每个结点的几何：x 坐标按宽度累进 */
const geo = computed(() => {
  const ns = view.value?.nodes ?? []
  const xs: number[] = []
  let x = 30
  for (const n of ns) { xs.push(x); x += nodeW(n) + GAP }
  return { xs, width: Math.max(x - GAP + 30, 150) }
})

const midY = TOP + NODE_H / 2
const nodeBottom = TOP + NODE_H
const VB_H = 152

const idxOf = (id: number | null | undefined) =>
  id == null ? -1 : (view.value?.nodes.findIndex(n => n.id === id) ?? -1)

function stepCls(i: number) {
  const s = props.step
  if (!s) return ''
  if (s.active && s.active.includes(i)) return 'active'
  if (s.highlights.includes(i)) return 'done'
  return ''
}
function nodeCls(n: LNNode, i: number) {
  const hl = n.highlight && n.highlight !== 'none' ? n.highlight : stepCls(i)
  return { node: true, active: hl === 'active', done: hl === 'done' }
}
/** 箭头高亮：两端任一 active 即热 */
function edgeHot(a: number, b: number) {
  const ns = view.value?.nodes ?? []
  const hl = (i: number) => i >= 0 && ns[i]?.highlight === 'active'
  return hl(a) || hl(b)
}

interface Path { d: string; hot: boolean; key: string; dashed?: boolean }

/** next 箭头：相邻直连 / 否则大弧绕上；null 画打地线 */
const nextPaths = computed<Path[]>(() => {
  const st = view.value
  if (!st) return []
  const out: Path[] = []
  st.nodes.forEach((n, i) => {
    const sx = geo.value.xs[i] + nodeW(n)
    if (n.next == null) {
      const ex = sx + 30
      out.push({ key: `n${n.id}`, d: `M ${sx} ${midY} L ${ex} ${midY}`, hot: false })
      out.push({ key: `g${n.id}`, d: `M ${ex - 8} ${midY + 9} L ${ex + 8} ${midY - 9}`, hot: false })
      return
    }
    const j = idxOf(n.next)
    if (j < 0) return
    const tx = geo.value.xs[j]
    const hot = edgeHot(i, j)
    if (j === i) {
      const cx = geo.value.xs[i] + nodeW(n) / 2
      out.push({ key: `n${n.id}`, d: `M ${cx - 14} ${TOP} C ${cx - 20} ${TOP - 40} ${cx + 20} ${TOP - 40} ${cx + 14} ${TOP}`, hot })
    } else if (j === i + 1) {
      out.push({ key: `n${n.id}`, d: `M ${sx} ${midY} L ${tx - 3} ${midY}`, hot })
    } else {
      const scx = geo.value.xs[i] + nodeW(n) / 2
      const tcx = geo.value.xs[j] + nodeW(st.nodes[j]) / 2
      const lift = j > i ? 34 : 44
      out.push({ key: `n${n.id}`, d: `M ${scx} ${TOP} C ${scx} ${TOP - lift} ${tcx} ${TOP - lift} ${tcx} ${TOP - 4}`, hot, dashed: !!n.ring })
    }
  })
  return out
})

/** prev 虚线箭头（双向链回指），一律走下方弧线 */
const prevPaths = computed<Path[]>(() => {
  const st = view.value
  if (!st) return []
  const out: Path[] = []
  st.nodes.forEach((n, i) => {
    if (n.prev == null) return
    const j = idxOf(n.prev)
    if (j < 0) return
    const scx = geo.value.xs[i] + CELL / 2
    const tcx = geo.value.xs[j] + nodeW(st.nodes[j]) - CELL / 2
    const hot = edgeHot(i, j)
    if (j === i) {
      out.push({ key: `p${n.id}`, d: `M ${scx} ${nodeBottom} C ${scx - 6} ${nodeBottom + 34} ${scx + 18} ${nodeBottom + 34} ${scx + 12} ${nodeBottom}`, hot, dashed: true })
    } else {
      const dip = Math.min(38, 20 + Math.abs(scx - tcx) * 0.12)
      out.push({ key: `p${n.id}`, d: `M ${scx} ${nodeBottom} C ${scx} ${nodeBottom + dip} ${tcx} ${nodeBottom + dip} ${tcx} ${nodeBottom + 2}`, hot, dashed: true })
    }
  })
  return out
})

const head = computed(() => {
  const st = view.value
  if (!st) return null
  const label = st.headLabel ?? 'head'
  const x0 = geo.value.xs[0] ?? 30
  if (st.headId == null) return { label, kind: 'null' as const, x: x0 }
  const j = idxOf(st.headId)
  if (j < 0) return { label, kind: 'null' as const, x: x0 }
  return { label, kind: 'ok' as const, x: x0, j }
})

const cellText = (n: LNNode) => (n.head ? '头结点' : n.value === '' ? '·' : String(n.value))
</script>

<template>
  <div v-if="view" class="ln">
    <div v-if="view.note" class="note">{{ view.note }}</div>
    <svg class="pic" :viewBox="`0 0 ${geo.width} ${VB_H}`" :style="{ maxWidth: geo.width * 1.35 + 'px' }" role="img">
      <defs>
        <marker id="ln-arrow" markerWidth="9" markerHeight="8" refX="8" refY="4" orient="auto" markerUnits="userSpaceOnUse">
          <path d="M0,0 L9,4 L0,8 z" style="fill: var(--vp-c-text-2)" />
        </marker>
        <marker id="ln-arrow-hot" markerWidth="9" markerHeight="8" refX="8" refY="4" orient="auto" markerUnits="userSpaceOnUse">
          <path d="M0,0 L9,4 L0,8 z" style="fill: var(--vp-c-brand-1)" />
        </marker>
      </defs>

      <!-- 外部头指针 -->
      <g v-if="head" class="headptr">
        <text :x="head.x - 34" :y="midY + 4" class="head-label">{{ head.label }}</text>
        <template v-if="head.kind === 'ok'">
          <line :x1="head.x - 12" :y1="midY" :x2="geo.xs[head.j] - 3" :y2="midY" class="edge" :class="{ hot: view.nodes[head.j].highlight === 'active' }" marker-end="url(#ln-arrow)" />
        </template>
        <template v-else>
          <line :x1="head.x - 12" :y1="midY" :x2="head.x + 18" :y2="midY" class="edge" />
          <line :x1="head.x + 10" :y1="midY + 9" :x2="head.x + 26" :y2="midY - 9" class="edge" />
        </template>
      </g>

      <!-- 结点 -->
      <g v-for="(n, i) in view.nodes" :key="n.id" :transform="`translate(${geo.xs[i]}, ${TOP})`" :class="nodeCls(n, i)">
        <text v-if="n.label" :x="nodeW(n) / 2" :y="-10" class="nlabel">{{ n.label }}</text>
        <rect class="frame" :width="nodeW(n)" :height="NODE_H" rx="8" />
        <line v-if="n.prev != null" class="divider" :x1="CELL" y1="0" :x2="CELL" :y2="NODE_H" />
        <line class="divider" :x1="nodeW(n) - CELL" y1="0" :x2="nodeW(n) - CELL" :y2="NODE_H" />
        <g v-if="n.prev != null" class="pcell"><rect class="cellbg" x="0" y="0" :width="CELL" :height="NODE_H" /><text class="celltxt dim" :x="CELL / 2" :y="midY - TOP + 4">prev</text></g>
        <g class="dcell"><rect class="cellbg" :x="nodeW(n) - CELL * 2" y="0" :width="CELL" :height="NODE_H" /><text class="celltxt" :x="nodeW(n) - CELL * 1.5" :y="midY - TOP + 5">{{ cellText(n) }}</text></g>
        <g class="ncell"><rect class="cellbg" :x="nodeW(n) - CELL" y="0" :width="CELL" :height="NODE_H" /><text class="celltxt dim" :x="nodeW(n) - CELL / 2" :y="midY - TOP + 4">next</text></g>
      </g>

      <!-- next 箭头 / 打地线 -->
      <path v-for="p in nextPaths" :key="p.key" :d="p.d" class="edge" :class="{ hot: p.hot, dashed: p.dashed }"
            :marker-end="p.hot ? 'url(#ln-arrow-hot)' : 'url(#ln-arrow)'" />
      <!-- prev 虚线箭头 -->
      <path v-for="p in prevPaths" :key="p.key" :d="p.d" class="edge prev" :class="{ hot: p.hot }"
            :marker-end="p.hot ? 'url(#ln-arrow-hot)' : 'url(#ln-arrow)'" />
    </svg>
    <div class="legend">
      <span class="chip"><i class="swatch active"></i>当前操作结点</span>
      <span class="chip"><i class="swatch done"></i>已完成</span>
      <span class="chip"><i class="line solid"></i>next 实线</span>
      <span class="chip"><i class="line dashed"></i>prev 虚线</span>
      <span class="chip"><i class="line ground"></i>NULL 打地</span>
    </div>
  </div>
</template>

<style scoped>
.ln { padding: 10px 0 4px; }
.note { font-size: 13px; margin-bottom: 6px; padding: 6px 10px; border-radius: 6px; background: var(--vp-c-bg-soft); display: inline-block; }
.pic { width: 100%; display: block; }

.node .frame { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); stroke-width: 1.4; transition: fill .25s, stroke .25s; }
.node.active .frame { stroke: var(--vp-c-yellow-1); fill: var(--vp-c-yellow-3); }
.node.done .frame { stroke: var(--vp-c-green-2); fill: var(--vp-c-green-3); }
.divider { stroke: var(--vp-c-divider); }
.cellbg { fill: transparent; }
.node.active .dcell .cellbg { fill: var(--vp-c-yellow-2); }
.node.done .dcell .cellbg { fill: var(--vp-c-green-2); }
.celltxt { font-size: 14px; font-weight: 600; text-anchor: middle; fill: var(--vp-c-text-1); font-variant-numeric: tabular-nums; }
.celltxt.dim { font-size: 10.5px; font-weight: 400; fill: var(--vp-c-text-3); }
.nlabel { font-size: 11.5px; text-anchor: middle; fill: var(--vp-c-text-2); }
.head-label { font-size: 12px; text-anchor: middle; fill: var(--vp-c-text-1); font-weight: 600; }

.edge { fill: none; stroke: var(--vp-c-text-2); stroke-width: 1.5; }
.edge.hot { stroke: var(--vp-c-brand-1); stroke-width: 2.2; }
.edge.prev { stroke-dasharray: 5 4; }
.edge.dashed { stroke-dasharray: 2 3; }

.legend { display: flex; gap: 12px; margin-top: 8px; font-size: 12px; opacity: .85; flex-wrap: wrap; align-items: center; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.active { background: var(--vp-c-yellow-2); border-color: var(--vp-c-yellow-1); }
.swatch.done { background: var(--vp-c-green-2); border-color: var(--vp-c-green-2); }
.line { display: inline-block; width: 22px; height: 0; vertical-align: 4px; }
.line.solid { border-top: 2px solid var(--vp-c-text-2); }
.line.dashed { border-top: 2px dashed var(--vp-c-text-2); }
.line.ground { border-top: 2px solid var(--vp-c-text-2); position: relative; }
</style>
