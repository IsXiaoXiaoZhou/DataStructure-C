// docs/.vitepress/theme/visualizer/steps/blockSearch.ts
import type { Step } from '../types'

// 分块查找（对应 08_查找/03_分块查找 blk_search.c）
// 语义：块间有序（前一块所有元素 < 后一块）、块内无序；
// 先在索引表上逐块比 max_key 定块，再进块顺序扫
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
    `全表 ${n} 个元素分 ${blocks.length} 块（每块 ${blockSize} 个）：块间有序（前一块 max ${maxes[0]} < 后一块最小值）、块内无序；先查索引表定块，再进块顺序扫`)]

  // 阶段一：索引表定块
  let bi = -1
  for (let i = 0; i < blocks.length; i++) {
    if (key > maxes[i]) {
      steps.push(frame(
        `key = ${key} > 块${i + 1} 的 max_key = ${maxes[i]} → 块${i + 1} 里全是 ≤ ${maxes[i]} 的数，不可能有 key，看下一块`,
        [], [starts[i], starts[i] + blocks[i].length - 1]))
    } else {
      bi = i
      steps.push(frame(
        `key = ${key} ≤ 块${i + 1} 的 max_key = ${maxes[i]} → 目标只可能在块${i + 1}（若存在）；索引定块用 ${i + 1} 次比较`,
        [], [starts[i], starts[i] + blocks[i].length - 1]))
      break
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
      `分块查找完成：索引 ${bi + 1} 次 + 块内 ${found - starts[bi] + 1} 次 = ${bi + 1 + found - starts[bi] + 1} 次比较；折中之美——比顺序查找快，比折半查找省"必须全有序"的要求`,
      [found], null))
  } else if (bi >= 0) {
    steps.push(frame(
      `块${bi + 1} 扫完没有 ${key} → 未找到（块内无序，只能顺序扫到底）；失败代价 = 定块 ${bi + 1} 次 + 块长 ${blocks[bi].length} 次`))
  } else {
    steps.push(frame(
      `key = ${key} 比所有块的 max_key 都大 → 未找到，索引表这一关就把它挡住了`))
  }
  return steps
}
