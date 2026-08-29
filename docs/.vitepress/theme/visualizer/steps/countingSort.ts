// docs/.vitepress/theme/visualizer/steps/countingSort.ts
import type { Step } from '../types'

// 值域 0~9；state 为桶形态 { array, buckets, bucketLabels }
export function countingSortSteps(input: number[]): Step[] {
  const a = [...input]
  const K = 10
  const buckets: number[][] = Array.from({ length: K }, () => [])
  const bucketLabels = Array.from({ length: K }, (_, k) => String(k))
  const frame = (arr: number[], narration: string, active: [number, number] | null = null, highlights: number[] = []): Step =>
    ({ state: { array: [...arr], buckets: buckets.map(b => [...b]), bucketLabels }, highlights, active, narration })
  const steps: Step[] = [frame(a, `初始数组 [${a.join(', ')}]；值域 0~9，用计数桶`)]
  for (let i = 0; i < a.length; i++) {
    buckets[a[i]].push(a[i])
    steps.push(frame(a, `C[${a[i]}]++：值 ${a[i]} 入桶 ${a[i]}`, [i, i]))
  }
  steps.push(frame(a, '计数完成，按桶序收集'))
  const out: number[] = []
  for (let v = 0; v < K; v++) {
    for (const _ of buckets[v]) {
      out.push(v)
      steps.push(frame([...out, ...remainingOf(a, out)], `桶 ${v} 收集 → out[${out.length - 1}] = ${v}`, [out.length - 1, out.length - 1], range(0, out.length)))
    }
  }
  steps.push({ state: { array: out, buckets: buckets.map(b => [...b]), bucketLabels }, highlights: out.map((_, k) => k), active: null, narration: `排序完成：[${out.join(', ')}]` })
  return steps
}

function remainingOf(a: number[], out: number[]): number[] {
  const need = new Map<number, number>()
  for (const v of out) need.set(v, (need.get(v) || 0) + 1)
  const rem: number[] = []
  for (const v of a) {
    const k = need.get(v) || 0
    if (k > 0) need.set(v, k - 1)  // 这个 v 已被收进出 out
    else rem.push(v)
  }
  return rem
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
