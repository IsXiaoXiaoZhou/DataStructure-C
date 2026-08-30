// docs/.vitepress/theme/visualizer/steps/circularQueue.ts
import type { Step } from '../types'
import type { StackQueueState } from './states'

// 循环顺序队列（对应 03_队列/01_循环顺序队列 seq_queue.c）
// 语义：data[0..MAX-1] 视为首尾相接的环；front 指队头元素、rear 指队尾元素的下一格；
//       判空 front == rear → DS_EMPTY(5)；判满 (rear+1)%MAX == front → DS_OVERFLOW(4)
//       （牺牲一格区分空满，实际容量 MAX-1）；进队出队各走一步取模回绕
//       演示取 MAX = 6（模块真身 SEQQUEUE_MAX_SIZE = 100）
export interface QueueOp { op: 'en' | 'de'; v?: number }
export interface CircularQueueInput { ops: QueueOp[] }

const MAX = 6

export function circularQueueSteps(input: CircularQueueInput): Step[] {
  const steps: Step[] = []
  const state = (data: (number | string)[], front: number, rear: number) => ({
    slots: [{
      label: `循环队列（演示 MAX=${MAX}，模块真身 100；rear 指队尾元素的下一格）`,
      kind: 'ring' as const,
      items: [...data],
      pointers: [{ name: 'front', index: front }, { name: 'rear', index: rear }]
    }]
  })
  const pushStep = (data: (number | string)[], front: number, rear: number, narration: string, active: number[] | null = null, highlights: number[] = []) => {
    steps.push({ state: state(data, front, rear) as StackQueueState, highlights, active, narration })
  }

  let data: (number | string)[] = new Array(MAX).fill('')
  let front = 0
  let rear = 0
  pushStep(data, front, rear, `初始：front = 0，rear = 0（判空 front == rear）；rear 指向队尾元素的下一格——若让 rear 直指队尾元素，空与满都退化成 front == rear 无法区分，牺牲一格才能两全，实际容量 ${MAX - 1}`)

  for (const op of input.ops) {
    if (op.op === 'en') {
      const v = op.v as number
      if ((rear + 1) % MAX === front) {
        pushStep(data, front, rear, `enqueue(${v}) 返回 DS_OVERFLOW（status=4）：判满 (rear+1)%${MAX} == front，即 (${rear}+1)%${MAX} = ${(rear + 1) % MAX} == ${front}——空余的最后一格是为区分空满保留的，不能写`)
        continue
      }
      data = [...data]
      data[rear] = v
      const old = rear
      rear = (rear + 1) % MAX
      pushStep(data, front, rear, `enqueue(${v})：data[${old}] = ${v}，rear = (rear+1)%${MAX} = (${old}+1)%${MAX} = ${rear}${old === MAX - 1 ? '（绕回队头，这就是循环队列的取模回绕）' : ''}`, [old])
    } else {
      if (front === rear) {
        pushStep(data, front, rear, `dequeue() 返回 DS_EMPTY（status=5）：判空 front == rear（${front} == ${rear}），队列为空`)
        continue
      }
      const v = data[front]
      const old = front
      data = [...data]
      data[old] = ''
      front = (front + 1) % MAX
      pushStep(data, front, rear, `dequeue() → ${v}：取 data[${old}] = ${v}，front = (front+1)%${MAX} = (${old}+1)%${MAX} = ${front}${old === MAX - 1 ? '（绕回队头）' : ''}`, [old])
    }
  }

  const size = ((rear - front + MAX) % MAX)
  pushStep(data, front, rear, `演示结束：元素个数 = (rear − front + ${MAX}) % ${MAX} = (${rear} − ${front} + ${MAX}) % ${MAX} = ${size}；+${MAX} 不是摆设——rear 绕到 front 前面时不加会算出负数`, null, data.map((v, i) => (v === '' ? -1 : i)).filter(i => i >= 0))
  return steps
}
