// docs/.vitepress/theme/visualizer/steps/insertionSort.ts
import type { Step } from '../types'

export function insertionSortSteps(input: number[]): Step[] {
  const a = [...input]
  const steps: Step[] = [{ state: [...a], highlights: [0], active: null, narration: `初始数组 [${a.join(', ')}]；认为 a[0] 已就位` }]
  for (let i = 1; i < a.length; i++) {
    const key = a[i]
    steps.push({ state: [...a], highlights: rangeExclusive(0, i), active: null, narration: `取出待插入元素 key = a[${i}] = ${key}` })
    let j = i - 1
    while (j >= 0 && a[j] > key) {
      steps.push({ state: [...a], highlights: [...rangeExclusive(0, i), i], active: [j, i], narration: `a[${j}] = ${a[j]} > key = ${key}，a[${j}] 后移一位` })
      a[j + 1] = a[j]
      steps.push({ state: [...a], highlights: [...rangeExclusive(0, i), i], active: [j, j + 1], narration: `后移完成 → [${a.join(', ')}]` })
      j--
    }
    if (j >= 0) steps.push({ state: [...a], highlights: [...rangeExclusive(0, i + 1)], active: [j, i], narration: `a[${j}] = ${a[j]} ≤ key，插入位置定为 ${j + 1}` })
    a[j + 1] = key
    steps.push({ state: [...a], highlights: rangeExclusive(0, i + 1), active: null, narration: `key 插入 a[${j + 1}] → 前 ${i + 1} 个元素有序` })
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function rangeExclusive(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
