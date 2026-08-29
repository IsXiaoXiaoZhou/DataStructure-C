// docs/.vitepress/theme/visualizer/steps/radixSort.ts
import type { Step } from '../types'

// 值域 0~99，LSD 基数排序（个位 → 十位）
export function radixSortSteps(input: number[]): Step[] {
  const a = [...input]
  const buckets: number[][] = Array.from({ length: 10 }, () => [])
  const bucketLabels = Array.from({ length: 10 }, (_, k) => String(k))
  const frame = (arr: number[], narration: string, active: number[] | null = null, highlights: number[] = []): Step =>
    ({ state: { array: [...arr], buckets: buckets.map(b => [...b]), bucketLabels }, highlights, active, narration })
  const steps: Step[] = [frame(a, `初始数组 [${a.join(', ')}]；LSD 基数排序：先个位后十位`)]

  for (const exp of [1, 10]) {
    const digitName = exp === 1 ? '个位' : '十位'
    for (let i = 0; i < a.length; i++) {
      const d = Math.floor(a[i] / exp) % 10
      buckets[d].push(a[i])
      steps.push(frame(a, `${digitName}趟第 ${i + 1}/${a.length} 个：${a[i]} 的 ${digitName}是 ${d}，入桶 ${d}`, [i]))
    }
    steps.push(frame(a, `${digitName}分发完成，按桶 0→9 依次收集`))
    const merged: number[] = []
    for (const b of buckets) { merged.push(...b); b.length = 0 }
    for (let i = 0; i < merged.length; i++) a[i] = merged[i]
    steps.push(frame(a, `${digitName}收集完成 → [${a.join(', ')}]`))
  }
  steps.push({ state: { array: [...a], buckets: buckets.map(b => [...b]), bucketLabels }, highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}
