// docs/.vitepress/theme/visualizer/steps/selectionSort.ts
import type { Step } from '../types'

export function selectionSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]` }]
  if (n < 2) {
    steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `长度 ${n} < 2，唯一元素本身即最小，无需选择` })
  }
  for (let i = 0; i < n - 1; i++) {
    let min = i
    for (let j = i + 1; j < n; j++) {
      steps.push({ state: [...a], highlights: range(0, i), active: [min, j], narration: `无序区最小暂为 a[${min}]=${a[min]}，与 a[${j}]=${a[j]} 比较` })
      if (a[j] < a[min]) {
        min = j
        steps.push({ state: [...a], highlights: range(0, i), active: [min], narration: `更小，最小下标更新为 ${min}` })
      }
    }
    if (min !== i) {
      ;[a[i], a[min]] = [a[min], a[i]]
      steps.push({ state: [...a], highlights: range(0, i + 1), active: [i, min], narration: `最小值 ${a[i]} 换到 a[${i}]` })
    } else {
      steps.push({ state: [...a], highlights: range(0, i + 1), active: null, narration: `a[${i}] 本就是无序区最小，不动` })
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
