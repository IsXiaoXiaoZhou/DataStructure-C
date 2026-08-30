// docs/.vitepress/theme/visualizer/steps/seqSearch.ts
import type { Step } from '../types'

// 顺序查找·哨兵版（对应 08_查找/01_顺序查找 seq_search.c 的 seq_search_sentinel）
// 语义：a[n] = key 预埋哨兵，循环只靠 a[i] != key 驱动；数据区命中即成功，
// 走到哨兵才停即失败；成功比较次数 = 位序，失败必扫 n+1 次
export interface SeqSearchInput { a: number[]; key: number }

export function seqSearchSteps(input: SeqSearchInput): Step[] {
  const { a, key } = input
  const n = a.length
  const arr: number[] = [...a, key] // 哨兵位
  const steps: Step[] = [{
    state: [...arr], highlights: [], active: null,
    narration: `待找 key = ${key}；先埋哨兵 a[${n}] = ${key}——兜底保证比到 n 必命中，循环里就省掉了每轮 i<n 的边界判断`
  }]

  let hit = n
  for (let i = 0; i <= n; i++) {
    if (arr[i] === key) { hit = i; break }
    steps.push({
      state: [...arr], highlights: [], active: [i],
      narration: `a[${i}] = ${arr[i]} ≠ ${key}，继续右移（已比较 ${i + 1} 次）`
    })
  }

  if (hit < n) {
    steps.push({
      state: [...arr], highlights: [hit], active: [hit],
      narration: `a[${hit}] = ${key} 命中！位序 ${hit + 1}，比较 ${hit + 1} 次`
    })
    steps.push({
      state: [...arr], highlights: [hit], active: null,
      narration: `查找成功：命中于位序 ${hit + 1}，比较 ${hit + 1} 次——哨兵版成功时比较次数恰为位序，n 次等概率下 ASL成功 = (n+1)/2 = ${(n + 1) / 2}`
    })
  } else {
    steps.push({
      state: [...arr], highlights: [], active: [n],
      narration: `a[${n}] = ${key}——比到哨兵才停下，说明数据区没有 ${key}，查找失败`
    })
    steps.push({
      state: [...arr], highlights: [], active: null,
      narration: `查找失败：比较 ${n + 1} 次（${n} 次数据 + 1 次哨兵命中）——失败必扫全表，这正是哨兵的代价与收益`
    })
  }
  return steps
}
