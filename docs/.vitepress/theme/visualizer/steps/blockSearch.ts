// docs/.vitepress/theme/visualizer/steps/blockSearch.ts
import type { Step } from '../types'

// 分块查找（对应 08_查找/03_分块查找 blk_search.c）
// 语义：块间有序（前一块所有元素 < 后一块）、块内无序；
// 阶段一在索引表上折半定块——对 max_key 数组做下界折半，找第一个 max_key >= key 的块
// （blk_search.c：while (lo <= hi) { m = lo+(hi-lo)/2; … }）；阶段二进块顺序扫
export interface BlockSearchInput { a: number[]; blockSize: number; key: number }

export function blockSearchSteps(input: BlockSearchInput): Step[] {
  const { a, blockSize, key } = input
  const blocks: number[][] = []
  for (let i = 0; i < a.length; i += blockSize) blocks.push(a.slice(i, i + blockSize))
  const maxes = blocks.map(b => Math.max(...b))
  const starts = blocks.map((_, i) => i * blockSize)
  const labels = blocks.map((b, i) => `块${i + 1} max=${Math.max(...b)}`)
  const n = a.length
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...a], buckets: blocks.map(b => [...b]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `全表 ${n} 个元素分 ${blocks.length} 块（每块 ${blockSize} 个）：块间有序（前一块 max ${maxes[0]} < 后一块最小值）、块内无序；先在索引表上折半定块（源码 blk_search 口径），再进块顺序扫`)]

  // 阶段一：索引折半定块（blk_search.c 的下界折半：找第一个 max_key >= key 的块）
  let bi = -1
  let idxCmp = 0
  {
    let lo = 0
    let hi = blocks.length - 1
    while (lo <= hi) {
      const m = lo + Math.floor((hi - lo) / 2)
      idxCmp++
      if (maxes[m] < key) {
        steps.push(frame(
          `索引折半第 ${idxCmp} 次：lo = ${lo}、hi = ${hi}、mid = ${m}，max_key = ${maxes[m]} < key = ${key} → 块${m + 1} 整块都 < ${key}，不可能有 key，丢掉左半：lo = ${m + 1}`,
          [], [starts[m], starts[m] + blocks[m].length - 1]))
        lo = m + 1
      } else {
        bi = m
        steps.push(frame(
          `索引折半第 ${idxCmp} 次：lo = ${lo}、hi = ${hi}、mid = ${m}，max_key = ${maxes[m]} ≥ key = ${key} → 块${m + 1} 可能装得下，记为候选，hi = ${m - 1} 继续向左压（要找的是第一个 max_key ≥ key 的块）`,
          [], [starts[m], starts[m] + blocks[m].length - 1]))
        if (m === 0) break
        hi = m - 1
      }
    }
  }

  // 阶段二：块内顺序扫
  let found = -1
  if (bi >= 0) {
    for (let k = 0; k < blocks[bi].length; k++) {
      const idx = starts[bi] + k
      if (a[idx] === key) {
        found = idx
        steps.push(frame(`块内扫到 a[${idx}] = ${key}，命中！`, [idx], [idx]))
        break
      }
      steps.push(frame(`块内 a[${idx}] = ${a[idx]} ≠ ${key}，继续`, [], [idx]))
    }
  }

  if (found >= 0) {
    steps.push(frame(
      `分块查找完成：索引折半 ${idxCmp} 次 + 块内 ${found - starts[bi] + 1} 次 = ${idxCmp + found - starts[bi] + 1} 次比较；折中之美——比顺序查找快，比折半查找省"必须全有序"的要求`,
      [found], null))
  } else if (bi >= 0) {
    steps.push(frame(
      `块${bi + 1} 扫完没有 ${key} → 未找到（块内无序，只能顺序扫到底）；失败代价 = 定块 ${idxCmp} 次 + 块长 ${blocks[bi].length} 次`))
  } else {
    steps.push(frame(
      `key = ${key} 比所有块的 max_key 都大（折半 ${idxCmp} 次后 lo > hi，候选块为空）→ 未找到，索引表这一关就把它挡住了`))
  }
  return steps
}
