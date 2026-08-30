<!-- docs/.vitepress/theme/visualizer/renderers/Graph.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

/**
 * Graph 渲染器 —— 固定布局图形态（环状布局/给定拓扑，坐标由生成器给定）。
 *
 * Step.state 契约：
 * {
 *   nodes: {
 *     id: number                 // 结点唯一 id（edges 的 from/to 必须引用存在的 id）
 *     label: string              // 结点内文字（如 "v0"、权值）
 *     x: number; y: number       // 0~100 坐标系（SVG viewBox 0 0 100 100 缩放）——布局由生成器负责
 *     highlight?: boolean        // 绿色（已入选/已访问/已确定）
 *     active?: boolean           // 橙色（当前考察），优先于 highlight
 *     color?: string             // 自定义填充色（CSS 颜色值），优先级最高
 *   }[]
 *   edges: {
 *     from: number; to: number   // 端点结点 id
 *     weight?: number            // 边中点标注的权值
 *     directed?: boolean         // 有向带箭头 / 无向一条线；两端互弧时自动画成一对小弧避让
 *     highlight?: boolean        // 绿色（已入选 MST/关键活动等）
 *     active?: boolean           // 橙色（当前考察的边），优先于 highlight
 *     label?: string             // 边中点小标签（weight 缺省时的替代文字）
 *   }[]
 *   aux?: { text: string }[]     // 辅助旁注区（dist/prev 数组、邻接链、队列内容…逐行文本）
 *   note?: string                // 顶部注释行（结论/算法状态）
 * }
 *
 * 绘图约定：结点半径固定；无向边一条线段、有向边端部画实心箭头（箭头按端点方向
 * 手工计算，不用 marker，避免多实例 id 冲突）；互弧对画成两条对称小弧；
 * 边文字带底色描边保证可读。高亮优先级：color > active（橙）> highlight（绿）。
 */
interface GNode { id: number; label: string; x: number; y: number; highlight?: boolean; active?: boolean; color?: string }
interface GEdge { from: number; to: number; weight?: number; directed?: boolean; highlight?: boolean; active?: boolean; label?: string }
interface GraphState { nodes: GNode[]; edges: GEdge[]; aux?: { text: string }[]; note?: string }

const props = defineProps<{ step: Step | null }>()

const view = computed<GraphState | null>(() => {
  if (!props.step) return null
  const o = props.step.state as GraphState
  if (o && Array.isArray(o.nodes)) return o
  return null
})

const R = 3.4          // 结点半径（viewBox 单位）
const ARROW = 2.1      // 箭头长度
const ARROW_W = 1.7    // 箭头半宽

const pos = computed<Map<number, GNode>>(() => {
  const m = new Map<number, GNode>()
  for (const n of view.value?.nodes ?? []) m.set(n.id, n)
  return m
})

const mutualKeys = computed<Set<string>>(() => {
  const s = new Set<string>()
  for (const e of view.value?.edges ?? []) {
    if (view.value!.edges.some(o => o.from === e.to && o.to === e.from)) s.add(`${e.from}-${e.to}`)
  }
  return s
})

interface EdgeGeom { key: string; d: string; head?: string; mx: number; my: number; text?: string; hot: 'active' | 'highlight' | null }
const substr = (x: number, l: number) => Math.min(Math.max(x, -l), l) // clamp helper

const edgeGeoms = computed<EdgeGeom[]>(() => {
  const st = view.value
  if (!st) return []
  const out: EdgeGeom[] = []
  for (const e of st.edges) {
    const a = pos.value.get(e.from), b = pos.value.get(e.to)
    if (!a || !b) continue
    const hot = e.active ? 'active' : (e.highlight ? 'highlight' : null)
    const text = e.weight !== undefined ? String(e.weight) : e.label
    const mutual = mutualKeys.value.has(`${e.from}-${e.to}`)
    const dx = b.x - a.x, dy = b.y - a.y
    const len = Math.hypot(dx, dy) || 1
    const ux = dx / len, uy = dy / len
    const px = -uy, py = ux                       // 垂直方向
    const bow = mutual ? 6 : 0                    // 互弧对的弯度
    const cx = (a.x + b.x) / 2 + px * bow
    const cy = (a.y + b.y) / 2 + py * bow
    const trimStart = R + 0.5
    const trimEnd = e.directed ? R + ARROW + 0.3 : R + 0.5
    let d: string, head: string | undefined, mx: number, my: number
    if (bow === 0) {
      const sx = a.x + ux * trimStart, sy = a.y + uy * trimStart
      const ex = b.x - ux * trimEnd, ey = b.y - uy * trimEnd
      d = `M ${sx} ${sy} L ${ex} ${ey}`
      mx = (sx + ex) / 2 - uy * 2.1
      my = (sy + ey) / 2 + ux * 2.1 + 0.9
      if (e.directed) {
        head = `M ${ex + ux * ARROW} ${ey + uy * ARROW} L ${ex - uy * ARROW_W} ${ey + ux * ARROW_W} L ${ex + uy * ARROW_W} ${ey - ux * ARROW_W} z`
      }
    } else {
      // 二次贝塞尔：端点沿端部切线方向缩进
      const t0x = substr(cx - a.x, 40), t0y = substr(cy - a.y, 40)
      const t1x = substr(b.x - cx, 40), t1y = substr(b.y - cy, 40)
      const l0 = Math.hypot(t0x, t0y) || 1, l1 = Math.hypot(t1x, t1y) || 1
      const sx = a.x + (t0x / l0) * trimStart, sy = a.y + (t0y / l0) * trimStart
      const ex = b.x - (t1x / l1) * trimEnd, ey = b.y - (t1y / l1) * trimEnd
      d = `M ${sx} ${sy} Q ${cx} ${cy} ${ex} ${ey}`
      mx = 0.25 * sx + 0.5 * cx + 0.25 * ex
      my = 0.25 * sy + 0.5 * cy + 0.25 * ey + 1.4
      if (e.directed) {
        const hx = t1x / l1, hy = t1y / l1
        head = `M ${ex + hx * ARROW} ${ey + hy * ARROW} L ${ex - hy * ARROW_W} ${ey + hx * ARROW_W} L ${ex + hy * ARROW_W} ${ey - hx * ARROW_W} z`
      }
    }
    out.push({ key: `${e.from}-${e.to}`, d, head, mx, my, text, hot })
  }
  return out
})

const nodeFont = (label: string) => (label.length <= 2 ? 3.0 : label.length === 3 ? 2.3 : 1.8)

function nodeCls(n: GNode) {
  return {
    node: true,
    custom: !!n.color,
    active: !n.color && !!n.active,
    highlight: !n.color && !n.active && !!n.highlight
  }
}
</script>

<template>
  <div v-if="view" class="gv">
    <div v-if="view.note" class="note">{{ view.note }}</div>

    <!-- 空图 -->
    <svg v-if="!view.nodes.length" class="pic" viewBox="0 0 100 40" role="img">
      <rect class="emptybox" x="38" y="12" width="24" height="14" rx="3" />
      <text class="emptytxt" x="50" y="21.5" text-anchor="middle">空图</text>
    </svg>

    <svg v-else class="pic" viewBox="0 0 100 100" role="img">
      <!-- 边在下、结点在上 -->
      <g v-for="e in edgeGeoms" :key="e.key">
        <path class="edge" :class="{ active: e.hot === 'active', highlight: e.hot === 'highlight' }" :d="e.d" />
        <path v-if="e.head" class="edge-head" :class="{ active: e.hot === 'active', highlight: e.hot === 'highlight' }" :d="e.head" />
        <text v-if="e.text" class="etext" :class="{ hot: !!e.hot }" :x="e.mx" :y="e.my" text-anchor="middle">{{ e.text }}</text>
      </g>
      <g v-for="n in view.nodes" :key="n.id" :class="nodeCls(n)">
        <circle class="ncircle" :cx="n.x" :cy="n.y" :r="R"
                :style="n.color ? { fill: n.color } : undefined" />
        <text class="ntext" :x="n.x" :y="n.y + nodeFont(n.label) * 0.36" text-anchor="middle"
              :style="{ fontSize: nodeFont(n.label) }">{{ n.label }}</text>
      </g>
    </svg>

    <!-- 辅助旁注区：dist/prev、邻接链、队列等 -->
    <div v-if="view.aux && view.aux.length" class="aux">
      <div v-for="(a, i) in view.aux" :key="i" class="auxline">{{ a.text }}</div>
    </div>

    <div class="legend">
      <span class="chip"><i class="swatch active"></i>当前考察</span>
      <span class="chip"><i class="swatch highlight"></i>已入选/已确定</span>
      <span class="chip"><i class="line"></i>边（有向带箭头）</span>
    </div>
  </div>
</template>

<style scoped>
.gv { padding: 10px 0 4px; }
.note { font-size: 13px; margin-bottom: 6px; padding: 6px 10px; border-radius: 6px; background: var(--vp-c-bg-soft); display: inline-block; }
.pic { width: 100%; max-width: 520px; display: block; margin: 0 auto; }

.edge { fill: none; stroke: var(--vp-c-text-2); stroke-width: .42; transition: stroke .25s; }
.edge.active, .edge-head.active { stroke: var(--vp-c-yellow-1); stroke-width: .7; }
.edge.highlight, .edge-head.highlight { stroke: var(--vp-c-green-2); stroke-width: .65; }
.edge-head { fill: var(--vp-c-text-2); stroke: none; }
.edge-head.active { fill: var(--vp-c-yellow-1); }
.edge-head.highlight { fill: var(--vp-c-green-2); }
.etext { font-size: 2.5px; fill: var(--vp-c-text-2); paint-order: stroke; stroke: var(--vp-c-bg); stroke-width: .9px; font-variant-numeric: tabular-nums; }
.etext.hot { fill: var(--vp-c-brand-1); font-weight: 700; }

.ncircle { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); stroke-width: .42; transition: fill .25s, stroke .25s; }
.node.active .ncircle { fill: var(--vp-c-yellow-2); stroke: var(--vp-c-yellow-1); }
.node.highlight .ncircle { fill: var(--vp-c-green-2); stroke: var(--vp-c-green-2); }
.ntext { fill: var(--vp-c-text-1); font-weight: 600; text-anchor: middle; }

.aux { margin: 10px auto 0; max-width: 520px; border: 1px dashed var(--vp-c-divider); border-radius: 8px; padding: 8px 12px; background: var(--vp-c-bg-soft); }
.auxline { font-size: 12.5px; font-family: var(--vp-font-family-mono, monospace); line-height: 1.7; white-space: pre-wrap; }

.emptybox { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); stroke-dasharray: 1.6 1.6; }
.emptytxt { font-size: 4px; fill: var(--vp-c-text-2); }

.legend { display: flex; gap: 12px; margin-top: 10px; font-size: 12px; opacity: .85; flex-wrap: wrap; align-items: center; }
.legend .chip { border: none; background: transparent; padding: 0; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.active { background: var(--vp-c-yellow-2); border-color: var(--vp-c-yellow-1); }
.swatch.highlight { background: var(--vp-c-green-2); border-color: var(--vp-c-green-2); }
.line { display: inline-block; width: 22px; height: 0; vertical-align: 4px; border-top: 2px solid var(--vp-c-text-2); }
</style>
