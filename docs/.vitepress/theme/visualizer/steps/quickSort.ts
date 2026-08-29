// docs/.vitepress/theme/visualizer/steps/quickSort.ts
import type { Step } from '../types'

export function quickSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；快排用显式栈做 Lomuto 分区` }]
  const stack: [number, number][] = [[0, n - 1]]
  while (stack.length) {
    const [lo, hi] = stack.pop()!
    if (lo >= hi) {
      if (lo === hi) steps.push({ state: [...a], highlights: [lo], active: null, narration: `区间 [${lo}, ${hi}] 单元素，天然有序` })
      continue
    }
    const pivot = a[hi]
    steps.push({ state: [...a], highlights: [hi], active: null, narration: `区间 [${lo}, ${hi}]：pivot = a[${hi}] = ${pivot}` })
    let i = lo
    for (let j = lo; j < hi; j++) {
      steps.push({ state: [...a], highlights: [hi, ...range(lo, i)], active: [j, hi], narration: `j=${j}：a[${j}]=${a[j]} 与 pivot=${pivot} 比较` })
      if (a[j] < pivot) {
        if (i !== j) {
          ;[a[i], a[j]] = [a[j], a[i]]
          steps.push({ state: [...a], highlights: [hi, ...range(lo, i + 1)], active: [i, j], narration: `小于 pivot，换到左侧 → [${a.join(', ')}]` })
        }
        i++
      }
    }
    if (i !== hi) {
      ;[a[i], a[hi]] = [a[hi], a[i]]
      steps.push({ state: [...a], highlights: [...range(lo, i), i, ...range(i + 1, hi + 1)], active: [i, hi], narration: `pivot 归位到 a[${i}] → [${a.join(', ')}]` })
    }
    steps.push({ state: [...a], highlights: [...range(lo, i), i, ...range(i + 1, hi + 1)], active: null, narration: `pivot ${pivot} 就位：左 [${lo}, ${i - 1}]，右 [${i + 1}, ${hi}] 入栈` })
    stack.push([lo, i - 1], [i + 1, hi])
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
