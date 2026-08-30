// docs/.vitepress/theme/visualizer/steps/dynaSeqGrow.ts
import type { Step } from '../types'

// 动态顺序表扩容搬家（对应 01_线性表/02_动态顺序表 dynamic_seq_list.c）
// 语义：seqlist_grow —— realloc(list->data, capacity*2) 倍增扩容；realloc 挪块时
// 自动保留原内容（数据已随扩容就位），旧块由 realloc 自行释放——源码没有拷贝循环、
// 也没有（也不可再有）显式 free 旧块；realloc 失败时原块不动，返回 DS_OVERFLOW
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
    `扩容：seqlist_grow 一句 realloc(list->data, 2×${n}) —— capacity ${n} → 2×${n} = ${newCap}；realloc 若在原块后找不到连续空间会另觅新块，并自动把原内容整体带过去（数据跟着走，不用手写拷贝），失败则原块不动、返回 DS_OVERFLOW`))

  for (let i = 0; i < n; i++) {
    fresh[i] = old[i]
    steps.push(frame(fresh, [old], [labelOld],
      `realloc 搬迁内部动作（示意）${i + 1}/${n}：new_data[${i}] = old_data[${i}] = ${old[i]}——原内容自动跟到新块，源码 seqlist_grow 里没有这个循环，也不用你写`, [i], [i, i]))
  }

  steps.push(frame(fresh, [[]], ['旧块（realloc 已回收）'],
    `扩容完成：旧块由 realloc 自行回收——源码没有任何一行显式 free 旧块的代码，也不可再补一刀 free（realloc 的返回值才是这块内存的唯一主人，乱 free 是 double-free）；指针改指新块，capacity ${n} → ${newCap}，length 仍为 ${n}`))

  steps.push(frame(fresh, [[]], ['旧块（realloc 已回收）'],
    `搬家完成：${n} 个元素全部住进 ${newCap} 格新块；倍增扩容把 n 次追加的总搬运代价摊成每次 O(1)——这就是"均摊"的来历`, fresh.slice(0, n).map((_, k) => k)))

  return steps
}
