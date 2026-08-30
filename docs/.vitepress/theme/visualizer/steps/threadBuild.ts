// docs/.vitepress/theme/visualizer/steps/threadBuild.ts
import type { Step } from '../types'
import { tstep, preNullBuild, bin, type TNode } from './treeKit'

// 中序线索化（对应 06_树/04_线索二叉树/thread_tree.c tt_inthreading）：
//   in_thread 与中序递归遍历同构，仅多两步"挂线索"（thread_visit）：
//     p->left == NULL  → ltag=1，left 指前驱（首结点挂 NULL）
//     pre->right == NULL → rtag=1，right 指后继
//   末结点后继线索置 NULL。n+1 个空链域回收 n-1 条线索 + 首尾 2 个 NULL。
//   线索不画成树边（tag 区分结构边与线索边，destroy 只走 tag==0 分支），
//   用 nodeLabel 标"前驱→X / 后继→X"、keyHighlight 标本帧新挂的线索键位。
export interface ThreadBuildInput { tokens: string[] }

interface ThNode { key: string; left: number | null; right: number | null; ltag: number; rtag: number }

export function threadBuildSteps(input: ThreadBuildInput): Step[] {
  const steps: Step[] = []
  const built = preNullBuild(input.tokens)
  if (!built || built.root == null) return steps // validate 已保证

  const nodes = new Map<number, ThNode>()
  built.nodes.forEach((nd, id) => nodes.set(id, { ...nd, ltag: 0, rtag: 0 }))
  const root = built.root

  // 中序序列（递归下降，此时尚无线索、只沿孩子指针）
  const inorder: number[] = []
  const walk = (id: number) => {
    const nd = nodes.get(id)!
    if (nd.ltag === 0 && nd.left != null) walk(nd.left)
    inorder.push(id)
    if (nd.rtag === 0 && nd.right != null) walk(nd.right)
  }
  walk(root)

  const keyOf = (id: number | null) => (id == null ? 'NULL' : nodes.get(id)!.key)

  /** 帧构造：nodeLabel 覆盖、本帧新挂线索的结点 keyHighlight */
  const frame = (labels: Record<number, string>, fresh: number[], narration: string, note?: string) => {
    const out: TNode[] = []
    const draw = (id: number) => {
      const nd = nodes.get(id)!
      out.push(bin(id, nd.key, nd.ltag === 0 ? nd.left : null, nd.rtag === 0 ? nd.right : null, {
        nodeLabel: labels[id],
        keyHighlight: fresh.includes(id) ? [0] : undefined
      }))
      if (nd.ltag === 0 && nd.left != null) draw(nd.left)
      if (nd.rtag === 0 && nd.right != null) draw(nd.right)
    }
    draw(root)
    steps.push(tstep(out, root, narration, note))
  }

  const labels: Record<number, string> = {}
  const refresh = () => {
    for (const [id, nd] of nodes) {
      const parts: string[] = []
      if (nd.ltag === 1) parts.push(`前驱→${keyOf(nd.left)}`)
      if (nd.rtag === 1) parts.push(`后继→${keyOf(nd.right)}`)
      labels[id] = parts.length ? parts.join(' ') : ''
    }
  }

  // 0. 开场
  const n = nodes.size
  const nullSlots = 2 * n - (n - 1) // 2n 指针域 - (n-1) 条边 = n+1
  frame(labels, [],
    `建树完成（${n} 结点）：n+1 = ${nullSlots} 个空链域可存 n-1 = ${n - 1} 条前驱后继线索（中序首结点无前驱、尾结点无后继，线索置 NULL）`,
    '二叉链表 + ltag/rtag 标志域：tag==0 指针是孩子，tag==1 指针是线索；线索化后中序遍历无需栈无需递归，空间 O(1)')

  // 1. 中序线索化：模拟 thread_visit 逐结点
  let pre: number | null = null
  inorder.forEach((id) => {
    const nd = nodes.get(id)!
    const fresh: number[] = []
    const parts: string[] = []
    if (nd.left == null) {
      nd.ltag = 1
      nd.left = pre
      fresh.push(id)
      parts.push(`${keyOf(id)} 左孩子为空 → ltag=1，left 指前驱 ${keyOf(pre)}${pre == null ? '（中序首结点）' : ''}`)
    } else {
      parts.push(`${keyOf(id)} 左孩子存在，ltag 保持 0`)
    }
    if (pre != null && nodes.get(pre)!.right == null) {
      const p = nodes.get(pre)!
      p.rtag = 1
      p.right = id
      if (!fresh.includes(pre)) fresh.push(pre)
      parts.push(`前驱 ${keyOf(pre)} 右孩子为空 → rtag=1，right 指向 ${keyOf(id)}（后继线索）`)
    }
    pre = id
    refresh()
    frame({ ...labels }, fresh, `中序到达 ${keyOf(id)}：${parts.join('；')}`)
  })

  // 2. 收尾：末结点后继置 NULL + 线索遍历说明
  refresh()
  const lastId = inorder[inorder.length - 1]
  const lastNd = nodes.get(lastId)!
  if (lastNd.right == null && lastNd.rtag === 0) {
    lastNd.rtag = 1 // 中序末结点: 后继线索置空（tt_inthreading 尾部）
  }
  refresh()
  const seq = inorder.map(id => keyOf(id)).join(' ')
  frame({ ...labels },
    `线索化完成：中序 ${seq}；此后中序遍历 = first（最左）→ 反复 next（线索直达或右子树最左），无需栈无需递归`,
    '求后继仅中序线索可以纯线索完成且不加父指针——教学以中序线索为主；线索会把树连成环，销毁必须靠 tag 区分结构边与线索边')

  return steps
}
