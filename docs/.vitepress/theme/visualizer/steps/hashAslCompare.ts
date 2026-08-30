// docs/.vitepress/theme/visualizer/steps/hashAslCompare.ts
import type { Step } from '../types'

// 散列查找性能分析（对应 08_查找/12_散列查找性能分析 hash_perf.c 的拉链法口径）
// 语义：除留余数 H(key) = key % p（p 取 ≤ n 的最大素数），桶内头插成链；
//   成功比较次数 = 结点在链中的序号（从 1 数）；失败比较次数 = 桶内结点数
//   （沿链比完遇 NULL）。理论（拉链法）：成功 ≈ 1+α/2，失败 ≈ α
export function hashAslCompareSteps(input: number[]): Step[] {
  const keys = [...input]
  const n = keys.length
  const p = largestPrimeAtMost(n)
  const buckets: number[][] = Array.from({ length: p }, () => [])
  const labels = Array.from({ length: p }, (_, k) => String(k))
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...keys], buckets: buckets.map(b => [...b]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `${n} 个关键字，表长取 p = ${p}（≤ n 的最大素数）；先把散列表建出来，再逐桶算"成功"与"失败"的平均查找长度`)]

  for (let i = 0; i < n; i++) {
    const h = keys[i] % p
    buckets[h].unshift(keys[i]) // 头插，与 hash_chain.c 一致
    steps.push(frame(
      `H(${keys[i]}) = ${keys[i]} % ${p} = ${h} → 头插入桶 ${h}（桶 ${h} 链：${buckets[h].join(' → ')}）`,
      [i], [i, i]))
  }

  // 成功 ASL：每桶沿链数序号
  let sumS = 0
  let cntS = 0
  for (let b = 0; b < p; b++) {
    for (let pos = 1; pos <= buckets[b].length; pos++) {
      const key = buckets[b][pos - 1]
      sumS += pos
      cntS++
      steps.push(frame(
        `成功查找 ${key}：在桶 ${b} 链中第 ${pos} 个 → 比较 ${pos} 次；已计 ${cntS}/${n} 个，累计 ${sumS} 次，当前 ASL成功 = ${sumS}/${cntS} = ${(sumS / cntS).toFixed(2)}`,
        [], null))
    }
  }

  // 失败 ASL：每桶沿链走到底
  let sumF = 0
  for (let b = 0; b < p; b++) {
    sumF += buckets[b].length
    steps.push(frame(
      `失败查找（桶 ${b}）：沿链比较 ${buckets[b].length} 次遇 NULL → 失败 ${buckets[b].length} 次；累计 ${sumF} 次 / ${p} 桶`,
      [], null))
  }

  steps.push(frame(
    `对比完成：ASL成功 = ${sumS}/${n} = ${(sumS / n).toFixed(2)}，ASL失败 = ${sumF}/${p} = ${(sumF / p).toFixed(2)}；装填因子 α = ${n}/${p} = ${(n / p).toFixed(2)}，理论（拉链法）：成功 ≈ 1+α/2，失败 ≈ α`,
    [], null))

  return steps
}

function largestPrimeAtMost(x: number): number {
  const isPrime = (v: number) => {
    if (v < 2) return false
    for (let d = 2; d * d <= v; d++) if (v % d === 0) return false
    return true
  }
  for (let v = Math.max(2, x); v >= 2; v--) if (isPrime(v)) return v
  return 2
}
