// docs/.vitepress/theme/visualizer/steps/hashChainInsert.ts
import type { Step } from '../types'

// 散列表·拉链法插入（对应 08_查找/10_散列表-拉链法 hash_chain.c 的 hc_insert）
// 语义：p 个桶各挂一条单链表，H(key) = key % p；新结点头插 O(1)，
// 不插重复键；桶内链序 = 头插序（后插的在链头）
export interface HashChainInsertInput { keys: number[]; p: number }

export function hashChainInsertSteps(input: HashChainInsertInput): Step[] {
  const { keys, p } = input
  const buckets: number[][] = Array.from({ length: p }, () => [])
  const labels = Array.from({ length: p }, (_, k) => String(k))
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...keys], buckets: buckets.map(b => [...b]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `拉链法：${p} 个桶各挂一条单链表；H(key) = key % ${p}，同余的键都挂到同一桶——冲突不再是"找下一个格子"，而是"在链上排一队"`)]

  for (let i = 0; i < keys.length; i++) {
    const key = keys[i]
    const h = key % p
    buckets[h].unshift(key) // 头插法
    steps.push(frame(
      `H(${key}) = ${key} % ${p} = ${h} → 头插入桶 ${h}：桶 ${h} 链变为 [${buckets[h].join(' → ')}]（新结点永远当链头，O(1)）`,
      [i], [i, i]))
  }

  const alpha = (keys.length / p).toFixed(2)
  steps.push(frame(
    `插入完成：${keys.length} 个结点分布在 ${p} 条链上，装填因子 α = ${keys.length}/${p} = ${alpha}；查找沿链扫，ASL成功 ≈ 1 + α/2——链越短越快，这就是"别把 α 弄大"的原因`))

  return steps
}
