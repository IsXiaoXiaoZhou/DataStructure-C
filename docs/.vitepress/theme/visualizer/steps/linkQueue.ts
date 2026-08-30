// docs/.vitepress/theme/visualizer/steps/linkQueue.ts
import type { Step } from '../types'
import type { LnNode, ListNodeState } from './states'

// 链队列入出队（对应 03_队列/02_链队列 link_queue.c）
// 语义：带头结点，front 恒指头结点、rear 恒指尾结点（空队 rear 同指头结点，判空 front == rear）；
//       入队尾插三步：rear->next = node; rear = node; length++；
//       出队：first = front->next 取值 → front->next = first->next →
//       若删的是最后一个结点必须 rear = front 回位头结点（否则 rear 悬空成野指针，
//       下一次 enqueue 就写已释放内存——本模块最关键的一步）→ free
export interface LinkQueueOp { op: 'en' | 'de'; v?: number }
export interface LinkQueueInput { ops: LinkQueueOp[] }

const HEAD_ID = 0

export function linkQueueSteps(input: LinkQueueInput): Step[] {
  const steps: Step[] = []

  /** data 值列表 → 结点数组；labels 覆盖 front/rear 标注 */
  const build = (vals: number[], labels: Record<number, string>, activeIdx = -1): LnNode[] => {
    const nodes: LnNode[] = [{ id: HEAD_ID, value: 'head', head: true, next: vals.length ? 1 : null, highlight: 'none', label: labels[HEAD_ID] }]
    vals.forEach((v, i) => {
      nodes.push({ id: i + 1, value: v, next: i + 1 < vals.length ? i + 2 : null, highlight: i === activeIdx ? 'active' : 'none', label: labels[i + 1] })
    })
    return nodes
  }
  const pushStep = (vals: number[], labels: Record<number, string>, narration: string, activeIdx = -1, extra: LnNode[] = [], nextFix: Record<number, number | null> = {}) => {
    const nodes = build(vals, labels, activeIdx)
    for (const n of nodes) if (n.id in nextFix) n.next = nextFix[n.id]
    steps.push({ state: { nodes: [...nodes, ...extra], headId: HEAD_ID, headLabel: 'front' } as ListNodeState, highlights: [], active: null, narration })
  }

  let vals: number[] = []
  let enCount = 0
  const nextEnId = () => 100 + enCount++
  pushStep(vals, {}, `初始：front 与 rear 同指头结点（判空 front == rear）；真正的队头元素在头结点之后——带头结点让出队也无需特判`)

  for (const op of input.ops) {
    if (op.op === 'en') {
      const v = op.v as number
      const id = nextEnId()
      // 悬停帧
      pushStep(vals, { [HEAD_ID]: vals.length ? 'front' : 'front=rear' }, `enqueue(${v})：malloc 新结点（value=${v}, next=NULL）`, -1,
        [{ id, value: v, next: null, highlight: 'none', label: '新结点' }])
      // rear->next = node
      pushStep(vals, { [HEAD_ID]: 'front', ...(vals.length ? { [vals.length]: 'rear' } : {}) }, `rear->next = node：新结点挂上链尾`, vals.length - 1,
        [{ id, value: v, next: null, highlight: 'active', label: 'node' }], vals.length ? { [vals.length]: id } : { [HEAD_ID]: id })
      // rear = node
      vals = [...vals, v]
      const labels: Record<number, string> = { [HEAD_ID]: 'front' }
      labels[vals.length] = 'rear'
      pushStep(vals, labels, `rear = node：rear 前移到新队尾（值 ${v}），length ${vals.length - 1} → ${vals.length}——尾插三步顺序无依赖但缺一不可`, vals.length - 1)
    } else {
      if (vals.length === 0) {
        pushStep(vals, { [HEAD_ID]: 'front=rear' }, `dequeue() 返回 DS_EMPTY（status=5）：front == rear（都指头结点），队列只剩头结点，为空`)
        continue
      }
      const v = vals[0]
      const wasRear = vals.length === 1
      // 取值帧
      pushStep(vals, { [HEAD_ID]: 'front', 1: 'first' }, `dequeue() → ${v}：first = front->next，取队头值 ${v}`, 0)
      // 跨接帧
      pushStep(vals, { [HEAD_ID]: 'front', 1: 'first' }, `front->next = first->next：头结点跨过被删结点`, -1, [], { [HEAD_ID]: vals.length > 1 ? 2 : null })
      // 删空回位特殊帧
      if (wasRear) {
        pushStep(vals, { [HEAD_ID]: 'front=rear' }, `删除的是最后一个结点：rear = front 拉回头结点——漏了这步 rear 悬空指向已释放内存，下一次 enqueue 就会写野指针`, -1, [], { [HEAD_ID]: null })
      }
      vals = vals.slice(1)
      const labels: Record<number, string> = { [HEAD_ID]: vals.length ? 'front' : 'front=rear' }
      if (vals.length) labels[vals.length] = 'rear'
      pushStep(vals, labels, `free(first)，length ${vals.length + 1} → ${vals.length}${wasRear ? '；队列删空后回到 front==rear 的初始形态，可继续复用' : ''}`)
    }
  }

  const labels: Record<number, string> = { [HEAD_ID]: vals.length ? 'front' : 'front=rear' }
  if (vals.length) labels[vals.length] = 'rear'
  pushStep(vals, labels, `演示结束：队头到队尾 [${vals.join(', ')}]${vals.length ? '' : '（空队，只剩头结点）'}；入队尾插、出队头删，各 O(1)`, -1, [], {})
  return steps
}
