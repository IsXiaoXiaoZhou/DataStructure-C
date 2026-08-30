// docs/.vitepress/theme/visualizer/steps/seqListRemove.ts
import type { Step } from '../types'

// 静态顺序表按位序删除（对应 01_线性表/01_静态顺序表 static_seq_list.c）
// 语义：删除第 pos 位后，data[pos..length-1] 整体前移一格补洞
export interface SeqListRemoveInput { list: number[]; pos: number }

export function seqListRemoveSteps(input: SeqListRemoveInput): Step[] {
  const { list, pos } = input
  const n = list.length
  const cap = Math.max(8, n + 2)
  const a: number[] = [...list, ...new Array(cap - n).fill(0)]
  const removed = list[pos - 1]
  const total = n - pos
  const steps: Step[] = [{
    state: [...a], highlights: [pos - 1], active: null,
    narration: `初始：length=${n}；删除第 ${pos} 位元素 data[${pos - 1}] = ${removed}，其后 ${total} 个元素逐个前移补洞`
  }]
  let moved = 0
  for (let i = pos - 1; i <= n - 2; i++) {
    a[i] = a[i + 1]
    moved++
    steps.push({
      state: [...a], highlights: [], active: [i, i + 1],
      narration: `第 ${moved}/${total} 步前移：data[${i + 1}] = ${list[i + 1]} → data[${i}]（与插入相反，删除从删除点向后逐个补）`
    })
  }
  a[n - 1] = 0
  steps.push({
    state: [...a], highlights: [], active: [n - 1, n - 1],
    narration: `末位 data[${n - 1}] 清 0（空槽），length ${n} → ${n - 1}`
  })
  steps.push({
    state: [...a], highlights: a.slice(0, n - 1).map((_, k) => k), active: null,
    narration: `删除完成：[${a.slice(0, n - 1).join(', ')}]，被删的是原第 ${pos} 位元素 ${removed}；共前移 ${total} 个元素，平均 (n-1)/2，删除同样是 O(n)`
  })
  return steps
}
