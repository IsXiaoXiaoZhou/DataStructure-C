// docs/.vitepress/theme/visualizer/steps/sharedStack.ts
import type { Step } from '../types'
import type { StackQueueState } from './states'

// 两栈共享空间（对应 02_栈/02_两栈共享空间 shared_stack.c）
// 语义：一段定长数组两栈相向使用——栈1 占 [0, top1] 向右增长（top1 初始 -1），
//       栈2 占 [top2, MAX-1] 向左增长（top2 初始 MAX）；判满 top1 + 1 == top2
//       （先于写入执行，保证两栈元素区间永不交叉）；各自判空 top1==-1 / top2==MAX
//       演示容量取 8（模块真身 SHAREDSTACK_MAX_SIZE = 100）
export interface SharedOp { id: 1 | 2; op: 'push' | 'pop'; v?: number }
export interface SharedStackInput { ops: SharedOp[] }

const CAP = 8

export function sharedStackSteps(input: SharedStackInput): Step[] {
  const steps: Step[] = []
  const state = (data: (number | string)[], top1: number, top2: number) => ({
    slots: [{
      label: `共享数组（演示 MAX=${CAP}，模块真身 100；(top1, top2) 开区间为共用空闲区）`,
      kind: 'shared' as const,
      items: [...data],
      pointers: [{ name: 'top1', index: top1 }, { name: 'top2', index: top2 }]
    }]
  })
  const pushStep = (data: (number | string)[], top1: number, top2: number, narration: string, active: number[] | null = null, highlights: number[] = []) => {
    steps.push({ state: state(data, top1, top2) as StackQueueState, highlights, active, narration })
  }

  let data: (number | string)[] = new Array(CAP).fill('')
  let top1 = -1
  let top2 = CAP
  pushStep(data, top1, top2, `初始：top1 = -1、top2 = ${CAP}（两栈各自"退到端点之外"），整段数组都是空闲区——为什么要共用？两个独立定长栈各占 ${CAP} 格时，任一栈满即溢出，共用方案只有整段用尽才报满`)

  for (const op of input.ops) {
    if (op.op === 'push') {
      const v = op.v as number
      if (top1 + 1 === top2) {
        const active = [top1, top2].filter(i => i >= 0 && i < CAP)
        pushStep(data, top1, top2, `栈${op.id} push(${v}) 返回 DS_OVERFLOW（status=4）：判满 top1 + 1 == top2（${top1} + 1 == ${top2}）成立——两栈合计占满整段数组，压谁都是溢出，这是本结构唯一要守的不变量`, active)
        continue
      }
      if (op.id === 1) {
        const old = top1
        top1++
        data = [...data]
        data[top1] = v
        pushStep(data, top1, top2, `栈1 push(${v})：top1 ${old} → ${top1}，data[${top1}] = ${v}（栈1 向右端扩张）；空闲区 (top1, top2) 还剩 ${top2 - top1 - 1} 格`, [top1])
      } else {
        const old = top2
        top2--
        data = [...data]
        data[top2] = v
        pushStep(data, top1, top2, `栈2 push(${v})：top2 ${old} → ${top2}，data[${top2}] = ${v}（栈2 向左端扩张）；空闲区还剩 ${top2 - top1 - 1} 格`, [top2])
      }
    } else {
      if (op.id === 1) {
        if (top1 === -1) {
          pushStep(data, top1, top2, `栈1 pop() 返回 DS_EMPTY（status=5）：栈1 空（top1 == -1）——栈1 的判空别顺手写成 top2 那套`, null, [])
          continue
        }
        const v = data[top1]
        const old = top1
        top1--
        data = [...data]
        data[old] = ''
        pushStep(data, top1, top2, `栈1 pop() → ${v}：先取 data[${old}] = ${v}，top1 ${old} → ${top1}`, null, top1 >= 0 ? [top1] : [])
      } else {
        if (top2 === CAP) {
          pushStep(data, top1, top2, `栈2 pop() 返回 DS_EMPTY（status=5）：栈2 空（top2 == ${CAP}）——栈2 判空是退回端点之外，不是 0`, null, [])
          continue
        }
        const v = data[top2]
        const old = top2
        top2++
        data = [...data]
        data[old] = ''
        pushStep(data, top1, top2, `栈2 pop() → ${v}：先取 data[${old}] = ${v}，top2 ${old} → ${top2}`, null, top2 < CAP ? [top2] : [])
      }
    }
  }

  const m1 = data.slice(0, top1 + 1).length
  const m2 = data.slice(top2).length
  pushStep(data, top1, top2, `演示结束：栈1 有 ${m1} 个（data[0..${top1}]）、栈2 有 ${m2} 个（data[${top2}..${CAP - 1}]），空闲 ${top2 - top1 - 1} 格；元素个数公式——栈1 = top1+1、栈2 = MAX-top2，两者之和 ≤ MAX`, null, [...data.keys()].filter(i => data[i] !== ''))
  return steps
}
