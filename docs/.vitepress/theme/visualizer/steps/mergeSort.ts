// docs/.vitepress/theme/visualizer/steps/mergeSort.ts
import type { Step } from '../types'

export function mergeSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；自底向上归并：宽 1,2,4…` }]
  const buf = new Array<number>(n)
  for (let width = 1; width < n; width *= 2) {
    for (let lo = 0; lo < n - width; lo += 2 * width) {
      const mid = lo + width, hi = Math.min(lo + 2 * width, n)
      steps.push({ state: [...a], highlights: range(lo, hi), active: null, narration: `合并有序段 [${lo}, ${mid - 1}] 与 [${mid}, ${hi - 1}]` })
      let i = lo, j = mid, k = lo
      while (i < mid && j < hi) {
        steps.push({ state: [...a], highlights: range(lo, hi), active: [i, j], narration: `比较 a[${i}]=${a[i]} 与 a[${j}]=${a[j]}，取${a[i] <= a[j] ? `a[${i}]` : `a[${j}]`}` })
        buf[k++] = a[i] <= a[j] ? a[i++] : a[j++]
      }
      while (i < mid) buf[k++] = a[i++]
      while (j < hi) buf[k++] = a[j++]
      for (let t = lo; t < hi; t++) a[t] = buf[t]
      steps.push({ state: [...a], highlights: range(lo, hi), active: null, narration: `段 [${lo}, ${hi - 1}] 归并完成 → [${a.join(', ')}]` })
    }
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
