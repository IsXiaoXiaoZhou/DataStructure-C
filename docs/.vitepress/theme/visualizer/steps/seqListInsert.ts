// docs/.vitepress/theme/visualizer/steps/seqListInsert.ts
import type { Step } from '../types'

// 静态顺序表插入（对应 01_线性表/01_静态顺序表 static_seq_list.c）
// 语义：data[0..length-1] 连续存放 a1..an；插入第 pos 位需把
// data[pos-1..length-1] 从尾向前整体后移一格，再写入新元素
export interface SeqListInsertInput { list: number[]; pos: number; value: number }

export function seqListInsertSteps(input: SeqListInsertInput): Step[] {
  const { list, pos, value } = input
  const n = list.length
  const cap = Math.max(8, n + 2)
  const a: number[] = [...list, ...new Array(cap - n).fill(0)]
  const total = n - pos + 1
  const opening = total === 0
    ? `初始：length=${n}，容量 ${cap}（尾部空槽显示 0）；位序 ${n + 1} 即表尾追加，无需移动元素，直接落位`
    : `初始：length=${n}，容量 ${cap}（尾部空槽显示 0）；在第 ${pos} 位插入 ${value}，需把 data[${pos - 1}..${n - 1}] 从尾到头后移一格，共 ${total} 个元素`
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: opening }]
  let moved = 0
  for (let i = n - 1; i >= pos - 1; i--) {
    a[i + 1] = a[i]
    moved++
    steps.push({
      state: [...a], highlights: [], active: [i, i + 1],
      narration: `第 ${moved}/${total} 步后移：data[${i}] = ${list[i]} → data[${i + 1}]（从尾向前挪，正向搬会覆盖未挪走的数据）`
    })
  }
  a[pos - 1] = value
  steps.push({
    state: [...a], highlights: [pos - 1], active: null,
    narration: `空位写入：data[${pos - 1}] = ${value}；length ${n} → ${n + 1}`
  })
  const filled = a.slice(0, n + 1)
  const closing = total === 0
    ? `插入完成：[${filled.join(', ')}]，第 ${pos} 位即 ${value}；表尾追加零移动——最好情况 O(1)`
    : `插入完成：[${filled.join(', ')}]，第 ${pos} 位即 ${value}；共后移 ${total} 个元素，平均移动 n/2，这就是顺序表插入 O(n) 的来源`
  steps.push({
    state: [...a], highlights: filled.map((_, k) => k), active: null,
    narration: closing
  })
  return steps
}
