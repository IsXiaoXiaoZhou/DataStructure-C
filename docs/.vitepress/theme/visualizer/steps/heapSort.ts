// docs/.vitepress/theme/visualizer/steps/heapSort.ts
import type { Step } from '../types'

export function heapSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；父 i 的孩子是 2i+1、2i+2` }]

  function siftDown(start: number, size: number) {
    let parent = start
    while (true) {
      const l = 2 * parent + 1, r = 2 * parent + 2
      let max = parent
      if (l < size && a[l] > a[max]) max = l
      if (r < size && a[r] > a[max]) max = r
      if (max === parent) {
        steps.push({ state: [...a], highlights: range(size, n), active: [parent], narration: `a[${parent}]=${a[parent]} 不小于孩子，下沉停止` })
        return
      }
      steps.push({ state: [...a], highlights: range(size, n), active: [parent, max], narration: `孩子 a[${max}]=${a[max]} 更大，与 a[${parent}]=${a[parent]} 交换` })
      ;[a[parent], a[max]] = [a[max], a[parent]]
      parent = max
    }
  }

  steps.push({ state: [...a], highlights: [], active: null, narration: '阶段一：自最后一个非叶节点起建大顶堆' })
  for (let i = Math.floor(n / 2) - 1; i >= 0; i--) siftDown(i, n)
  steps.push({ state: [...a], highlights: [], active: null, narration: `建堆完成 → [${a.join(', ')}]` })
  for (let size = n - 1; size > 0; size--) {
    steps.push({ state: [...a], highlights: range(size + 1, n), active: [0, size], narration: `堆顶 ${a[0]} 与末尾 a[${size}]=${a[size]} 交换，堆大小缩为 ${size}` })
    ;[a[0], a[size]] = [a[size], a[0]]
    steps.push({ state: [...a], highlights: range(size, n), active: [0], narration: `a[${size}] 就位，新堆顶下沉调整` })
    siftDown(0, size)
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
