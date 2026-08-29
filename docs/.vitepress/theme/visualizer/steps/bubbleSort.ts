// docs/.vitepress/theme/visualizer/steps/bubbleSort.ts
import type { Step } from '../types'

export function bubbleSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]，共 ${n} 个元素` }]
  const done = () => a.map((_, k) => k)
  for (let i = 0; i < n - 1; i++) {
    let swapped = false
    for (let j = 0; j < n - 1 - i; j++) {
      steps.push({ state: [...a], highlights: range(n - i, n), active: [j, j + 1], narration: `第 ${i + 1} 轮：比较 a[${j}]=${a[j]} 与 a[${j + 1}]=${a[j + 1]}` })
      if (a[j] > a[j + 1]) {
        ;[a[j], a[j + 1]] = [a[j + 1], a[j]]
        swapped = true
        steps.push({ state: [...a], highlights: range(n - i, n), active: [j, j + 1], narration: `逆序，交换 → [${a.join(', ')}]` })
      }
    }
    if (!swapped) {
      steps.push({ state: [...a], highlights: done(), active: null, narration: '本轮无交换，已整体有序，提前结束' })
      break
    }
    steps.push({ state: [...a], highlights: i === n - 2 ? done() : range(n - 1 - i, n), active: null, narration: `第 ${i + 1} 轮结束，最大值冒泡到位（尾部 ${i + 1} 个已有序）` })
  }
  steps.push({ state: [...a], highlights: done(), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
