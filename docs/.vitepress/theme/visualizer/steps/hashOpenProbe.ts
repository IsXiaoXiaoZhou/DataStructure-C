// docs/.vitepress/theme/visualizer/steps/hashOpenProbe.ts
import type { Step } from '../types'

// 开放定址法·二次探测插入（对应 08_查找/11_散列表-开放定址法 hash_open.c）
// 语义：H0 = key % p；冲突时按教材二次探测序列
//   di = 1², −1², 2², −2², … 交替正负，Hi = (H0 + di) mod p（负则加 p 转正）
export interface HashOpenProbeInput { keys: number[]; p: number }

export function hashOpenProbeSteps(input: HashOpenProbeInput): Step[] {
  const { keys, p } = input
  const table: (number | null)[] = new Array(p).fill(null)
  const buckets: number[][] = Array.from({ length: p }, () => [])
  const labels = Array.from({ length: p }, (_, k) => String(k))
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...keys], buckets: buckets.map(b => [...b]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `二次探测散列表：表长 m = ${p}（取 4k+3 型素数，探测序列才能覆盖全表）；H(key) = key % ${p}，冲突时按 +1², −1², +2², −2² 交替找空格`)]

  for (let i = 0; i < keys.length; i++) {
    const key = keys[i]
    const h0 = key % p
    if (table[h0] === null) {
      table[h0] = key
      buckets[h0].push(key)
      steps.push(frame(
        `H₀(${key}) = ${key} % ${p} = ${h0}，格 ${h0} 是空的 → 直接落位`,
        [i], [i, i]))
      continue
    }
    steps.push(frame(
      `H₀(${key}) = ${key} % ${p} = ${h0}，已被 ${table[h0]} 占用，开始探测`,
      [i], [i, i]))
    for (let t = 1; ; t++) {
      const k = Math.ceil(t / 2)
      const d = (t % 2 === 1 ? 1 : -1) * k * k
      const idx = ((h0 + d) % p + p) % p
      const sign = d >= 0 ? `+${k}²` : `−${k}²`
      if (table[idx] === null) {
        table[idx] = key
        buckets[idx].push(key)
        steps.push(frame(
          `d${t} = ${sign} → H${t} = (${h0} ${d >= 0 ? '+' : '−'} ${k * k}) % ${p} = ${idx}，格 ${idx} 空 → ${key} 落位（探测 ${t} 次）`,
          [i], [i, i]))
        break
      }
      steps.push(frame(
        `d${t} = ${sign} → H${t} = (${h0} ${d >= 0 ? '+' : '−'} ${k * k}) % ${p} = ${idx}，仍被 ${table[idx]} 占，继续`,
        [i], [i, i]))
    }
  }

  const alpha = (keys.length / p).toFixed(2)
  steps.push(frame(
    `插入完成：${keys.length}/${p} 格占用，装填因子 α = ${alpha}；α 越大堆积越重、探测越长，查找期望 O(1/(1−α))——这是开放定址的死穴`))

  return steps
}
