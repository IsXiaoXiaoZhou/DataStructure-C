<!-- docs/.vitepress/theme/visualizer/renderers/StackQueue.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

/**
 * StackQueue 渲染器 —— 栈 / 队列 / 共享栈 / 环形队列形态。
 *
 * Step.state 契约：
 * {
 *   slots: {
 *     label: string                       // 槽标题（如"顺序栈""男队"）
 *     items: (number|string)[]            // 格子内容；'' 表示空格（环槽 / 共享槽 / 容量占位格）
 *     capacity?: number                   // 声明容量：渲染器在 items 尾部补空格占位到 capacity；
 *                                         //   items 数 == capacity 时全部实格加"已满"暗色描边
 *     kind?: 'stack'|'queue'|'shared'|'ring'
 *                                         // stack: 竖排、栈顶在上（items[0] 为栈底）
 *                                         // queue/shared: 横排（items[0] 在最左）
 *                                         // ring: SVG 圆排布（items.length 即环长 MAX，标注取模回绕）
 *     pointers?: { name: string; index: number }[]
 *                                         // 指针标签（top/front/rear/top1/top2…）；
 *                                         //   index 为本槽 items 的下标（0 起），
 *                                         //   -1 表示指向槽外左端/下端（如空栈 top=-1），
 *                                         //   items.length 表示右端/上端之外（如共享栈空栈2 top2=MAX）
 *   }[]
 * }
 *
 * 高亮契约（Step.active / Step.highlights）：
 *   数字为「全局格子索引」——把 slots 依序展平编号：
 *   slot0 的 items 占 [0, L0)，slot1 接着占 [L0, L0+L1)……
 *   生成器只管按全局索引写高亮，渲染器负责映射回（槽, 槽内下标）。
 */
interface Pointer { name: string; index: number }
interface Slot {
  label: string
  items: (number | string)[]
  capacity?: number
  kind?: 'stack' | 'queue' | 'shared' | 'ring'
  pointers?: Pointer[]
}

const props = defineProps<{ step: Step | null }>()

const view = computed<Slot[] | null>(() => {
  if (!props.step) return null
  const o = props.step.state as { slots?: Slot[] }
  if (o && Array.isArray(o.slots) && o.slots.length) return o.slots
  return null
})

/** 指针配色（按全 slots 中出现顺序分配） */
const PALETTE = ['var(--vp-c-brand-1)', 'var(--vp-c-purple-2)', 'var(--vp-c-green-2)', 'var(--vp-c-red-2)']
const pointerColor = computed<Record<string, string>>(() => {
  const map: Record<string, string> = {}
  let k = 0
  for (const s of view.value ?? []) for (const p of s.pointers ?? []) {
    if (!(p.name in map)) map[p.name] = PALETTE[k++ % PALETTE.length]
  }
  return map
})

/** 全局高亮 → 每槽 [起, 止) 前缀；返回某槽的 Set（局部下标） */
function localIdx(slotIdx: number, which: 'active' | 'highlights'): Set<number> {
  const out = new Set<number>()
  const s = props.step
  if (!s) return out
  const list = which === 'active' ? (s.active ?? []) : s.highlights
  let offset = 0
  for (let i = 0; i < slotIdx; i++) offset += (view.value?.[i]?.items.length ?? 0)
  for (const g of list) if (g >= offset && g < offset + (view.value?.[slotIdx]?.items.length ?? 0)) out.add(g - offset)
  return out
}

function cellCls(slot: Slot, slotIdx: number, i: number, filled: boolean) {
  const s = props.step
  if (!s || !filled) return { cell: true, empty: true }
  const isActive = localIdx(slotIdx, 'active').has(i)
  const isDone = localIdx(slotIdx, 'highlights').has(i)
  const isFull = !!slot.capacity && (slot.items.length >= slot.capacity)
  return { cell: true, active: isActive, done: isDone, full: isFull && !isActive && !isDone }
}

/** 横排槽：把指针按列分桶；-1 → 左端外，items.length → 右端外 */
function chipCols(slot: Slot, slotIdx: number): { pre: Pointer[]; cols: Pointer[][]; post: Pointer[] } {
  const pre: Pointer[] = [], post: Pointer[] = []
  const cols: Pointer[][] = (slot.items ?? []).map(() => [])
  for (const p of slot.pointers ?? []) {
    if (p.index === -1) pre.push(p)
    else if (p.index >= cols.length) post.push(p)
    else cols[p.index].push(p)
  }
  void slotIdx
  return { pre, cols, post }
}

/** 竖排栈槽：栈顶指针在 -1 时放到底部空位，items.length 时放到顶部空位 */
function chipRows(slot: Slot, slotIdx: number): { pre: Pointer[]; rows: Pointer[][]; post: Pointer[] } {
  return chipCols(slot, slotIdx)
}

// ---------- ring 布局（SVG 圆排布） ----------
const R_MAX = 6 // 视觉上圆排布最多 10 格，超出退化直排（本批动画 MAX=6）
function isRing(slot: Slot) { return slot.kind === 'ring' && slot.items.length <= R_MAX }

const RING = { cx: 200, cy: 152, r: 104, cellW: 52, cellH: 42 }
function ringPos(slot: Slot, i: number) {
  const n = slot.items.length
  const ang = (-90 + (i * 360) / n) * (Math.PI / 180)
  return { x: RING.cx + RING.r * Math.cos(ang), y: RING.cy + RING.r * Math.sin(ang), angDeg: -90 + (i * 360) / n }
}
/** 指针标签位置：比格子更外圈，按指针序号错开角度避免重叠 */
function ringChipPos(slot: Slot, slotIdx: number, p: Pointer) {
  const n = slot.items.length
  const pi = Math.max(0, (slot.pointers ?? []).indexOf(p))
  const spread = n > 2 ? Math.min(14, 60 / n) : 18
  const ang = (ringPos(slot, ((p.index % n) + n) % n).angDeg + (pi - ((slot.pointers?.length ?? 1) - 1) / 2) * spread) * (Math.PI / 180)
  const r = RING.r + 40
  return { x: RING.cx + r * Math.cos(ang), y: RING.cy + r * Math.sin(ang), cx: ringPos(slot, ((p.index % n) + n) % n).x, cy: ringPos(slot, ((p.index % n) + n) % n).y }
}
function ringCellCls(slot: Slot, slotIdx: number, i: number) {
  return cellCls(slot, slotIdx, i, true)
}

const pad = (v: number | string) => (v === '' ? '·' : String(v))
</script>

<template>
  <div v-if="view" class="sq">
    <div class="slots">
    <div v-for="(slot, si) in view" :key="si" class="slot" :class="'kind-' + (slot.kind ?? 'queue')">
      <div class="slot-label">{{ slot.label }}</div>

      <!-- 环形：SVG 圆排布 -->
      <svg v-if="isRing(slot)" class="ring" :viewBox="`0 0 400 304`" role="img">
        <circle class="ring-guide" :cx="RING.cx" :cy="RING.cy" :r="RING.r" />
        <g v-for="(v, i) in slot.items" :key="i" :class="ringCellCls(slot, si, i)">
          <rect class="cell-rect" :x="ringPos(slot, i).x - RING.cellW / 2" :y="ringPos(slot, i).y - RING.cellH / 2"
                :width="RING.cellW" :height="RING.cellH" rx="7" />
          <text class="cell-val" :x="ringPos(slot, i).x" :y="ringPos(slot, i).y + 2">{{ pad(v) }}</text>
          <text class="cell-idx" :x="ringPos(slot, i).x" :y="ringPos(slot, i).y + 15">{{ i }}</text>
        </g>
        <g v-for="(p, pi) in slot.pointers ?? []" :key="'p' + pi">
          <line class="pointer-tick" :x1="ringChipPos(slot, si, p).x" :y1="ringChipPos(slot, si, p).y"
                :x2="ringChipPos(slot, si, p).cx" :y2="ringChipPos(slot, si, p).cy" />
          <g class="ring-chip" :style="{ color: pointerColor[p.name] }">
            <rect :x="ringChipPos(slot, si, p).x - 27" :y="ringChipPos(slot, si, p).y - 11" width="54" height="22" rx="11" />
            <text :x="ringChipPos(slot, si, p).x" :y="ringChipPos(slot, si, p).y + 4">{{ p.name }}={{ ((p.index % slot.items.length) + slot.items.length) % slot.items.length }}</text>
          </g>
        </g>
        <text class="ring-note" :x="RING.cx" :y="RING.cy + 4">MAX = {{ slot.items.length }}</text>
      </svg>

      <!-- 栈：竖排，栈顶在上 -->
      <div v-else-if="slot.kind === 'stack'" class="stackwrap">
        <div class="chipzone post">
          <span v-for="(p, pi) in chipRows(slot, si).post" :key="pi" class="chip" :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span>
        </div>
        <div class="stackrow">
          <div class="chipcol">
            <div v-for="(p, i) in chipRows(slot, si).rows" :key="i" class="chipzone cellw">
              <span v-for="(pp, k) in p" :key="k" class="chip" :style="{ color: pointerColor[pp.name], borderColor: pointerColor[pp.name] }">{{ pp.name }}={{ pp.index }}</span>
            </div>
          </div>
          <div class="cellcol">
            <!-- column-reverse：DOM 先出现的画在下面 → items[0]（栈底）在最下、栈顶在上 -->
            <div v-for="(v, i) in slot.items" :key="i" :class="cellCls(slot, si, i, true)">
              <span class="val">{{ pad(v) }}</span>
              <span class="idx">{{ i }}</span>
            </div>
            <div v-for="e in Math.max(0, (slot.capacity ?? slot.items.length) - slot.items.length)" :key="'e' + e"
                 class="cell empty"><span class="val">·</span></div>
          </div>
        </div>
        <div class="chipzone pre">
          <span v-for="(p, pi) in chipRows(slot, si).pre" :key="pi" class="chip" :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span>
        </div>
      </div>

      <!-- 队列 / 共享数组：横排 -->
      <div v-else class="hwrap">
        <div class="chiprow">
          <div class="chipzone cellw pre"><span v-for="(p, pi) in chipCols(slot, si).pre" :key="pi" class="chip" :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span></div>
          <div v-for="(col, i) in chipCols(slot, si).cols" :key="i" class="chipzone cellw">
            <span v-for="(p, pi) in col" :key="pi" class="chip" :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span>
          </div>
          <div v-for="e in Math.max(0, (slot.capacity ?? slot.items.length) - slot.items.length)" :key="'pad' + e" class="chipzone cellw"></div>
          <div class="chipzone cellw post"><span v-for="(p, pi) in chipCols(slot, si).post" :key="pi" class="chip" :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span></div>
        </div>
        <div class="cellrow">
          <div v-for="(v, i) in slot.items" :key="i" :class="cellCls(slot, si, i, v !== '')">
            <span class="val">{{ pad(v) }}</span>
            <span class="idx">{{ i }}</span>
          </div>
          <div v-for="e in Math.max(0, (slot.capacity ?? slot.items.length) - slot.items.length)" :key="'e' + e"
               class="cell empty"><span class="val">·</span></div>
        </div>
      </div>
    </div>
    </div>

    <div class="legend">
      <span class="chip"><i class="swatch active"></i>当前操作</span>
      <span class="chip"><i class="swatch done"></i>已完成/已配对</span>
      <span class="chip"><i class="swatch"></i>未涉及</span>
      <span class="chip"><i class="swatch empty"></i>空格</span>
      <template v-for="(c, name) in pointerColor" :key="name">
        <span class="chip"><i class="swatch pointer" :style="{ background: c }"></i>{{ name }}</span>
      </template>
    </div>
  </div>
</template>

<style scoped>
.sq { padding: 12px 0; }
.slots { display: flex; flex-direction: column; gap: 18px; }
.slot { border: 1px solid var(--vp-c-divider); border-radius: 8px; padding: 10px 14px 12px; background: var(--vp-c-bg); }
.slot-label { font-size: 12px; font-weight: 600; opacity: .75; margin-bottom: 8px; }

.cellw { width: 46px; }
.cell, .cellw { min-height: 20px; }
.chipzone { display: flex; align-items: center; justify-content: center; gap: 3px; min-height: 22px; }
.chip { font-size: 11px; line-height: 1; padding: 3px 6px; border: 1px solid var(--vp-c-divider); border-radius: 10px; background: var(--vp-c-bg-soft); white-space: nowrap; }

/* 横排（队列/共享） */
.cellrow { display: flex; gap: 6px; }
.chiprow { display: flex; gap: 6px; margin-bottom: 2px; }
.cellrow .cell { width: 46px; display: flex; flex-direction: column; align-items: center; justify-content: center; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg-soft); min-height: 44px; transition: background .25s, border-color .25s; }

/* 竖排（栈） */
.stackrow { display: flex; gap: 8px; align-items: stretch; }
.cellcol { display: flex; flex-direction: column-reverse; gap: 6px; }
.chipcol { display: flex; flex-direction: column-reverse; gap: 6px; }
.cellcol .cell { width: 46px; display: flex; flex-direction: column; align-items: center; justify-content: center; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg-soft); min-height: 40px; }
.stackwrap .pre, .stackwrap .post { justify-content: center; }

/* 三态与空格 */
.cell.active, .cellrow .cell.active { border-color: var(--vp-c-yellow-1); background: var(--vp-c-yellow-2); }
.cell.done, .cellrow .cell.done { border-color: var(--vp-c-green-2); background: var(--vp-c-green-2); }
.cell.full, .cellrow .cell.full { border-color: var(--vp-c-danger-2); }
.cell.empty, .cellrow .cell.empty { border-style: dashed; opacity: .5; background: transparent; }
.val { font-size: 13px; font-weight: 600; font-variant-numeric: tabular-nums; }
.idx { font-size: 10px; opacity: .55; }

/* 环形 SVG */
.ring { width: 100%; max-width: 400px; display: block; margin: 0 auto; }
.ring-guide { fill: none; stroke: var(--vp-c-divider); stroke-dasharray: 4 5; }
.ring .cell-rect { fill: var(--vp-c-bg-soft); stroke: var(--vp-c-divider); transition: fill .25s, stroke .25s; }
.ring g.active .cell-rect { fill: var(--vp-c-yellow-2); stroke: var(--vp-c-yellow-1); }
.ring g.done .cell-rect { fill: var(--vp-c-green-2); stroke: var(--vp-c-green-2); }
.ring .cell-val { font-size: 14px; font-weight: 600; text-anchor: middle; fill: var(--vp-c-text-1); font-variant-numeric: tabular-nums; }
.ring .cell-idx { font-size: 9px; text-anchor: middle; fill: var(--vp-c-text-2); opacity: .7; }
.ring .ring-note { font-size: 12px; text-anchor: middle; fill: var(--vp-c-text-2); }
.pointer-tick { stroke: var(--vp-c-text-3); stroke-dasharray: 3 3; }
.ring-chip rect { fill: var(--vp-c-bg); stroke: currentColor; }
.ring-chip text { font-size: 12px; text-anchor: middle; fill: currentColor; }

/* 图例 */
.legend { display: flex; gap: 12px; margin-top: 12px; font-size: 12px; opacity: .85; flex-wrap: wrap; align-items: center; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.done { background: var(--vp-c-green-2); border-color: var(--vp-c-green-2); }
.swatch.active { background: var(--vp-c-yellow-2); border-color: var(--vp-c-yellow-1); }
.swatch.empty { background: transparent; border-style: dashed; }
.swatch.pointer { border: none; }
</style>
