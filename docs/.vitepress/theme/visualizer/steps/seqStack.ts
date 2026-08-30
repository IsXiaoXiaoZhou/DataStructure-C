// docs/.vitepress/theme/visualizer/steps/seqStack.ts
import type { Step } from '../types'
import type { StackQueueState } from './states'

// 顺序栈 push/pop（对应 02_栈/01_顺序栈 seq_stack.c）
// 语义：data[0..top] 连续存放，top 指向栈顶元素（空栈 -1）；
//       进栈先自增再存值（data[++top] = v），出栈先取值再自减；
//       判满 top == MAX-1 → DS_OVERFLOW(4)，判空 top == -1 → DS_EMPTY(5)
//       演示容量取 6（模块真身 SEQSTACK_MAX_SIZE = 100，画面画不下）
export interface StackOp { op: 'push' | 'pop'; v?: number }
export interface SeqStackInput { ops: StackOp[] }

const CAP = 6

export function seqStackSteps(input: SeqStackInput): Step[] {
  const steps: Step[] = []
  const state = (items: number[], top: number) => ({
    slots: [{
      label: `顺序栈（演示容量 ${CAP}，模块真身 100）`,
      kind: 'stack' as const,
      capacity: CAP,
      items: [...items],
      pointers: [{ name: 'top', index: top }]
    }]
  })
  const pushStep = (items: number[], top: number, narration: string, active: number[] | null = null, highlights: number[] = []) => {
    steps.push({ state: state(items, top) as StackQueueState, highlights, active, narration })
  }

  let items: number[] = []
  let top = -1
  pushStep(items, top, `初始：空栈，top = -1（本实现 top 指向栈顶元素而非下一空位，判空条件是 top == -1）；判满条件则是 top == ${CAP - 1}`)

  for (const op of input.ops) {
    if (op.op === 'push') {
      const v = op.v as number
      if (top === CAP - 1) {
        pushStep(items, top, `push(${v}) 返回 DS_OVERFLOW（status=4）：判满 top == ${CAP - 1} 成立，栈已满，无法压入——定长数组的宿命`, [top])
        continue
      }
      const old = top
      items = [...items, v]
      top = items.length - 1
      pushStep(items, top, `push(${v})：top 先自增（${old} → ${top}），再存值 data[${top}] = ${v}——"先加后存"与判满条件是一套约定，别混用两派`, [top])
    } else {
      if (top === -1) {
        pushStep(items, top, `pop() 返回 DS_EMPTY（status=5）：判空 top == -1 成立，栈为空，无法弹出`, null, [])
        continue
      }
      const v = items[items.length - 1]
      const old = top
      items = items.slice(0, -1)
      top = items.length - 1
      pushStep(items, top, `pop() → ${v}：先取值 data[${old}] = ${v}，top 再下移（${old} → ${top}）——顺序颠倒会读到栈外元素`, null, top >= 0 ? [top] : [])
    }
  }

  pushStep(items, top, `演示结束：自栈底到栈顶 [${items.join(', ')}]，top = ${top}，栈内 ${items.length} 个元素；进栈/出栈/取栈顶全部只动栈顶那一格，O(1)`, null, items.map((_, i) => i))
  return steps
}
