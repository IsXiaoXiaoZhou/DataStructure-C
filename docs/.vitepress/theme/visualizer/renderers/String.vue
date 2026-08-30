<!-- docs/.vitepress/theme/visualizer/renderers/String.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

/**
 * String 渲染器 —— 串 / 双行文本指针形态（BF/KMP 双行、定长/堆分配单行、块链分块、外排多行）。
 *
 * Step.state 契约：
 * {
 *   rows: {
 *     label: string                        // 行标题（如"主串 S""模式串 T""块 0（ch[0..3]）"）
 *     chars: (string|number)[]             // 字符格内容；'' 表示空格（渲染为 ·）
 *     capacity?: number                    // 声明容量：渲染器在 chars 尾部补空格占位到 capacity
 *                                          //   （定长缓冲 / 堆块容量演示）
 *     pointers?: { name: string; index: number; offset?: number }[]
 *                                          // 指针小标签（i/j/k/游标名…）：画在本行格子上方，
 *                                          //   index 为本行 chars 的下标（0 起），-1 表示行首之外、
 *                                          //   chars.length 表示行尾之外；同一格多指针按 offset
 *                                          //   自上而下错开（0 最靠近格子），未给 offset 按出现序
 *     charHighlight?: number[]             // chars 下标：绿色（已匹配 / 已定）
 *     charActive?: number[]                // chars 下标：橙色（当前比较 / 当前写入），优先级最高
 *     strike?: number[]                    // chars 下标：灰色删除线（跳过 / 失配回退段 / 已释放）
 *   }[]
 *   note?: string                          // 顶部注释行（结论 / next 表旁注 / EOF 语义）
 * }
 *
 * 高亮优先级：charActive（橙）> charHighlight（绿）> strike（灰删除线）。
 * 本渲染器只读 state 内的行级高亮字段，不消费 Step.highlights / Step.active。
 */
interface SPointer { name: string; index: number; offset?: number }
interface SRow {
  label: string
  chars: (string | number)[]
  capacity?: number
  pointers?: SPointer[]
  charHighlight?: number[]
  charActive?: number[]
  strike?: number[]
}
interface StringState { rows: SRow[]; note?: string }

const props = defineProps<{ step: Step | null }>()

const view = computed<StringState | null>(() => {
  if (!props.step) return null
  const o = props.step.state as StringState
  if (o && Array.isArray(o.rows)) return o
  return null
})

/** 指针配色（按全 rows 中出现顺序分配，同名字同色） */
const PALETTE = ['var(--vp-c-brand-1)', 'var(--vp-c-purple-2)', 'var(--vp-c-green-2)', 'var(--vp-c-red-2)', 'var(--vp-c-yellow-1)']
const pointerColor = computed<Record<string, string>>(() => {
  const map: Record<string, string> = {}
  let k = 0
  for (const r of view.value?.rows ?? []) for (const p of r.pointers ?? []) {
    if (!(p.name in map)) map[p.name] = PALETTE[k++ % PALETTE.length]
  }
  return map
})

/** 某行的指针按列分桶：-1 → 行首外，≥len → 行尾外，其余入所在列；桶内按 offset 排序 */
function chipCols(row: SRow): { pre: SPointer[]; cols: SPointer[][]; post: SPointer[] } {
  const pre: SPointer[] = [], post: SPointer[] = []
  const cols: SPointer[][] = row.chars.map(() => [])
  for (const p of row.pointers ?? []) {
    if (p.index === -1) pre.push(p)
    else if (p.index >= cols.length) post.push(p)
    else cols[p.index].push(p)
  }
  const byOffset = (a: SPointer, b: SPointer) => (a.offset ?? 0) - (b.offset ?? 0)
  cols.forEach(c => c.sort(byOffset))
  pre.sort(byOffset)
  post.sort(byOffset)
  return { pre, cols, post }
}

function cellCls(row: SRow, i: number, filled: boolean) {
  const isActive = (row.charActive ?? []).includes(i)
  const isDone = (row.charHighlight ?? []).includes(i)
  const isStrike = (row.strike ?? []).includes(i)
  return {
    cell: true,
    active: filled && isActive,
    done: filled && isDone && !isActive,
    strike: isStrike && !isActive && !isDone,
    empty: !filled
  }
}

const pad = (v: string | number) => (v === '' ? '·' : String(v))
</script>

<template>
  <div v-if="view" class="vs">
    <div v-if="view.note" class="note">{{ view.note }}</div>
    <div v-if="!view.rows.length" class="emptyrow">（空串：尚无字符/结块）</div>
    <div class="rows">
      <div v-for="(row, ri) in view.rows" :key="ri" class="row">
        <div class="row-label">{{ row.label }}</div>
        <div class="grid">
          <div class="chiprow">
            <div class="chipzone cellw pre"><span v-for="(p, pi) in chipCols(row).pre" :key="pi" class="chip"
              :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span></div>
            <div v-for="(col, i) in chipCols(row).cols" :key="i" class="chipzone cellw">
              <span v-for="(p, pi) in col" :key="pi" class="chip"
                :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span>
            </div>
            <div v-for="e in Math.max(0, (row.capacity ?? row.chars.length) - row.chars.length)" :key="'pad' + e" class="chipzone cellw"></div>
            <div class="chipzone cellw post"><span v-for="(p, pi) in chipCols(row).post" :key="pi" class="chip"
              :style="{ color: pointerColor[p.name], borderColor: pointerColor[p.name] }">{{ p.name }}={{ p.index }}</span></div>
          </div>
          <div class="cellrow">
            <div v-for="(v, i) in row.chars" :key="i" :class="cellCls(row, i, v !== '')">
              <span class="val" :class="{ struck: (row.strike ?? []).includes(i) }">{{ pad(v) }}</span>
              <span class="idx">{{ i }}</span>
            </div>
            <div v-for="e in Math.max(0, (row.capacity ?? row.chars.length) - row.chars.length)" :key="'e' + e" class="cell empty">
              <span class="val">·</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="legend">
      <span class="chip"><i class="swatch active"></i>当前比较/写入</span>
      <span class="chip"><i class="swatch done"></i>已匹配/已定</span>
      <span class="chip"><i class="swatch strike"></i>回退/跳过/已释放</span>
      <span class="chip"><i class="swatch empty"></i>空格</span>
      <template v-for="(c, name) in pointerColor" :key="name">
        <span class="chip"><i class="swatch pointer" :style="{ background: c }"></i>{{ name }}</span>
      </template>
    </div>
  </div>
</template>

<style scoped>
.vs { padding: 12px 0; }
.note { font-size: 13px; margin-bottom: 10px; padding: 6px 10px; border-radius: 6px; background: var(--vp-c-bg-soft); display: inline-block; }
.emptyrow { font-size: 13px; opacity: .6; padding: 8px 2px; }
.rows { display: flex; flex-direction: column; gap: 14px; }
.row { border: 1px solid var(--vp-c-divider); border-radius: 8px; padding: 10px 14px 12px; background: var(--vp-c-bg); }
.row-label { font-size: 12px; font-weight: 600; opacity: .75; margin-bottom: 8px; }

.cellw { width: 40px; }
.cellw, .cell { min-height: 20px; }
.chipzone { display: flex; align-items: flex-end; justify-content: center; gap: 3px; min-height: 22px; }
.chip { font-size: 11px; line-height: 1; padding: 3px 6px; border: 1px solid var(--vp-c-divider); border-radius: 10px; background: var(--vp-c-bg-soft); white-space: nowrap; }
.chiprow { display: flex; gap: 5px; margin-bottom: 2px; }
.cellrow { display: flex; gap: 5px; }
.cellrow .cell { width: 40px; display: flex; flex-direction: column; align-items: center; justify-content: center; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg-soft); min-height: 44px; transition: background .25s, border-color .25s; }
.cell.active { border-color: var(--vp-c-yellow-1); background: var(--vp-c-yellow-2); }
.cell.done { border-color: var(--vp-c-green-2); background: var(--vp-c-green-2); }
.cell.strike { border-color: var(--vp-c-divider); background: var(--vp-c-bg-soft); opacity: .45; }
.cell.empty { border-style: dashed; opacity: .5; background: transparent; }
.val { font-size: 13px; font-weight: 600; font-variant-numeric: tabular-nums; }
.val.struck { text-decoration: line-through 1.5px var(--vp-c-text-3); }
.idx { font-size: 10px; opacity: .55; }

.legend { display: flex; gap: 12px; margin-top: 12px; font-size: 12px; opacity: .85; flex-wrap: wrap; align-items: center; }
.legend .chip { border: none; background: transparent; padding: 0; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.done { background: var(--vp-c-green-2); border-color: var(--vp-c-green-2); }
.swatch.active { background: var(--vp-c-yellow-2); border-color: var(--vp-c-yellow-1); }
.swatch.strike { opacity: .5; }
.swatch.empty { background: transparent; border-style: dashed; }
.swatch.pointer { border: none; }
</style>
