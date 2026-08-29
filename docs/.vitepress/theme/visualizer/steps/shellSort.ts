// docs/.vitepress/theme/visualizer/steps/shellSort.ts
import type { Step } from '../types'

export function shellSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；希尔排序按增量分组做插入排序` }]
  for (let gap = Math.floor(n / 2); gap > 0; gap = Math.floor(gap / 2)) {
    steps.push({ state: [...a], highlights: [], active: null, narration: `增量 gap = ${gap}：相距 ${gap} 的元素为一组` })
    for (let i = gap; i < n; i++) {
      const key = a[i]
      let j = i - gap
      while (j >= 0 && a[j] > key) {
        steps.push({ state: [...a], highlights: [], active: [j, j + gap], narration: `gap=${gap}：比较 a[${j}]=${a[j]} 与 key=${key}，a[${j}] 后移` })
        a[j + gap] = a[j]
        j -= gap
      }
      if (j + gap !== i) steps.push({ state: [...a], highlights: [], active: [j + gap, i], narration: `gap=${gap}：key=${key} 插入 a[${j + gap}]` })
      a[j + gap] = key
      steps.push({ state: [...a], highlights: [], active: null, narration: `gap=${gap} 本步完成 → [${a.join(', ')}]` })
    }
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}
