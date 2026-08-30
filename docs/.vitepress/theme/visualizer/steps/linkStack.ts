// docs/.vitepress/theme/visualizer/steps/linkStack.ts
import type { Step } from '../types'
import type { LnNode, ListNodeState } from './states'

// 链栈 push/pop（对应 02_栈/03_链栈 link_stack.c）
// 语义：无头结点，top 即链表头指针（栈顶 = 首结点，栈空 = top == NULL）；
//       进栈头插两步：node->next = top; top = node；出栈：取值、top 后移、free。
//       链栈无判满（结点按需 malloc）；判空 top == NULL → DS_EMPTY(5)
export interface LinkStackOp { op: 'push' | 'pop'; v?: number }
export interface LinkStackInput { ops: LinkStackOp[] }

export function linkStackSteps(input: LinkStackInput): Step[] {
  // 纯函数要求 id 分配确定性：按 push 次序静态预分配（100 起避免与数据值混淆）
  const ids: number[] = []
  {
    let k = 100
    for (const op of input.ops) if (op.op === 'push') ids.push(k++)
  }

  const steps: Step[] = []
  const pushStep = (nodes: LnNode[], headId: number | null, narration: string, active: number[] | null = null, highlights: number[] = [], note?: string) => {
    steps.push({ state: { nodes, headId, headLabel: 'top', note } as ListNodeState, highlights, active, narration })
  }

  let nodes: LnNode[] = []
  let headId: number | null = null
  let pushCount = 0
  pushStep(nodes, headId, `初始空栈：top == NULL——链栈不需要头结点（头插法不存在"首结点之前插入"的特判），也不需要判满（结点按需 malloc）`)

  for (const op of input.ops) {
    if (op.op === 'push') {
      const v = op.v as number
      const id = ids[pushCount++]
      const oldTop = nodes[0] ?? null
      // 悬停帧
      pushStep([...nodes, { id, value: v, next: null, highlight: 'none', label: '新结点' }], headId,
        `push(${v})：malloc 新结点（value=${v}, next=NULL），即将头插到链头`)
      // 第 1 步：node->next = top
      pushStep(
        [...nodes.map(n => ({ ...n, highlight: 'none' as const })), { id, value: v, next: headId, highlight: 'active' as const }],
        headId,
        `第 1 步 node->next = top：新结点接管原栈顶${oldTop ? `（${oldTop.value}）` : '（原栈为空，next 保持 NULL）'}——先接管，头指针还按兵不动`
      )
      // 第 2 步：top = node
      nodes = [{ id, value: v, next: headId, highlight: 'active' }, ...nodes.map(n => ({ ...n, highlight: 'none' as const }))]
      headId = id
      pushStep(nodes, headId, `第 2 步 top = node：${v} 成为新栈顶（头插 O(1)，size++）——写反两步顺序，原整条链就挂在新结点自己的 next 上成自环`)
    } else {
      if (headId === null) {
        pushStep(nodes, headId, `stack_pop 返回 DS_EMPTY（status=5）：栈为空（top == NULL），无法弹出`)
        continue
      }
      const topNode = nodes[0]
      const v = topNode.value
      // 取值帧
      pushStep(nodes.map((n, i) => (i === 0 ? { ...n, highlight: 'active' as const, label: '栈顶' } : { ...n, highlight: 'none' as const })), headId,
        `pop() → ${v}：先取栈顶结点的值 ${v}（此刻结点还在链上）`)
      // 摘链释放帧
      const rest = nodes.slice(1).map(n => ({ ...n, highlight: 'none' as const }))
      nodes = rest
      headId = topNode.next ?? null
      pushStep(nodes, headId, `top 改指原栈顶的后继${headId !== null ? `（值 ${nodes[0]?.value}）` : '（NULL，栈空）'}，再 free 结点——先摘链再释放，顺序颠倒就是悬垂访问`)
    }
  }

  pushStep(nodes, headId, `演示结束：自栈顶向下 [${nodes.map(n => n.value).join(', ')}]${nodes.length ? '，栈底最后弹出' : '（空栈）'}——后进先出，进出栈都只在链头 O(1)`, null, nodes.map((_, i) => i))
  return steps
}
