// docs/.vitepress/theme/visualizer/steps/hashFuncMap.ts
import type { Step } from '../types'

// 散列函数·除留余数法映射（对应 08_查找/09_散列函数 hash_func.c 的 hf_division）
// 语义：H(key) = key % p，p 取 <= 表长的素数；逐个关键字算 H 并入桶
export interface HashFuncMapInput { keys: number[]; p: number }

export function hashFuncMapSteps(input: HashFuncMapInput): Step[] {
  const { keys, p } = input
  const buckets: number[][] = Array.from({ length: p }, () => [])
  const labels = Array.from({ length: p }, (_, k) => String(k))
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...keys], buckets: buckets.map(b => [...b]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `除留余数法 H(key) = key % ${p}（p 取 ≤ 表长的素数，余数分布才均匀，见 hf_division）；下面逐个关键字算 H 并入桶`)]

  for (let i = 0; i < keys.length; i++) {
    const h = keys[i] % p
    buckets[h].push(keys[i])
    steps.push(frame(
      `H(${keys[i]}) = ${keys[i]} % ${p} = ${h} → 入桶 ${h}`,
      [i], [i, i]))
  }

  // 收尾按实际输入动态找一对同余撞桶的关键字（不硬编码默认数据的例子）
  const clashBucket = buckets.findIndex(b => b.length >= 2)
  const clashText = clashBucket >= 0
    ? `${buckets[clashBucket][0]} 和 ${buckets[clashBucket][1]} 同余（都余 ${clashBucket}）撞进同一桶`
    : '本轮输入恰好各余不同、没撞桶'
  steps.push(frame(
    `映射完成：${keys.length} 个关键字各归各桶，H 值域 [0, ${p})——${clashText}；关键字个数逼近 p 时冲突不可避免，处理交给拉链法/开放定址法`))

  return steps
}
