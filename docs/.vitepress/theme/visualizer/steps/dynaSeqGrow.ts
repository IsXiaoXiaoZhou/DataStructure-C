// docs/.vitepress/theme/visualizer/steps/dynaSeqGrow.ts
import type { Step } from '../types'

// 动态顺序表扩容搬家（对应 01_线性表/02_动态顺序表 dynamic_seq_list.c）
// 语义：seqlist_grow —— new_capacity = capacity * 2，逐元素搬到新块后
// 释放旧块；倍增策略使尾部追加均摊 O(1)
export function dynaSeqGrowSteps(input: number[]): Step[] {
  const old = [...input]
  const n = old.length
  const newCap = n * 2
  const labelOld = `旧表（容量 ${n}）`
  const frame = (arr: number[], buckets: number[][], labels: string[], narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...arr], buckets: buckets.map(b => [...b]), bucketLabels: [...labels] }, highlights, active, narration })

  const steps: Step[] = [frame(old, [old], [labelOld],
    `表已满：length = ${n} = capacity = ${n}，再追加必须扩容——静态表遇到这一步只能返回 DS_OVERFLOW，动态表的选择是搬家`)]

  const fresh: number[] = new Array(newCap).fill(0)
  steps.push(frame(fresh, [old], [labelOld],
    `扩容：按倍增策略开 2×${n} = ${newCap} 的新块 new_data（realloc 在原块后无连续空间时会另觅新块，数据不会自动跟过去）`))

  for (let i = 0; i < n; i++) {
    fresh[i] = old[i]
    steps.push(frame(fresh, [old], [labelOld],
      `搬运 ${i + 1}/${n}：new_data[${i}] = old_data[${i}] = ${old[i]}`, [i], [i, i]))
  }

  steps.push(frame(fresh, [[]], ['旧表（已 free）'],
    `free(old_data) 释放旧块，指针改指新块：capacity ${n} → ${newCap}，length 仍为 ${n}`))

  steps.push(frame(fresh, [[]], ['旧表（已 free）'],
    `搬家完成：${n} 个元素全部住进 ${newCap} 格新块；倍增扩容把 n 次追加的总搬运代价摊成每次 O(1)——这就是"均摊"的来历`, fresh.slice(0, n).map((_, k) => k)))

  return steps
}
