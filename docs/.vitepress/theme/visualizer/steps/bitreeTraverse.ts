// docs/.vitepress/theme/visualizer/steps/bitreeTraverse.ts
import type { Step } from '../types'
import { tstep, preNullBuild, traverseOrder, bin, type TNode } from './treeKit'

// 二叉树遍历（对应 06_树/03_二叉树链式实现/bitree.c 的 7 种遍历）：
//   输入 "pre:A,B,#,..." / "in:..." / "post:..." / "level:..."——段前缀选模式，
//   其余为带空标记（#）的先序串（level 模式为不带标记的层序串），与 bitree_create 同构。
//   递归三版只差"访问根的时机"：先序进左子树前、中序左子树归来后、后序两棵子树都归来；
//   层序把栈换成队列。四模式共用本生成器（参数化）。
export type TraverseMode = 'pre' | 'in' | 'post' | 'level'
export interface TraverseInput { mode: TraverseMode; tokens: string[] }

const MODE_NAME: Record<TraverseMode, string> = { pre: '先序', in: '中序', post: '后序', level: '层序' }

export function bitreeTraverseSteps(input: TraverseInput): Step[] {
  const steps: Step[] = []
  const { mode, tokens } = input

  // 建树：先序+# 递归建（level 模式按层序建完全树）；leftOf/rightOf 保左右槽位语义
  const keys: string[] = []
  const leftOf = new Map<number, number | null>()
  const rightOf = new Map<number, number | null>()
  const childrenMap = new Map<number, number[]>()
  let root: number | null = null
  if (mode === 'level') {
    tokens.forEach((_, i) => {
      keys[i] = tokens[i]
      const l = 2 * i + 1 < tokens.length ? 2 * i + 1 : null
      const r = 2 * i + 2 < tokens.length ? 2 * i + 2 : null
      leftOf.set(i, l)
      rightOf.set(i, r)
      childrenMap.set(i, [l, r].filter((x): x is number => x != null))
    })
    root = tokens.length ? 0 : null
  } else {
    const built = preNullBuild(tokens)
    if (!built || built.root == null) return steps // validate 已保证不发生
    root = built.root
    built.nodes.forEach((nd, id) => {
      keys[id] = nd.key
      leftOf.set(id, nd.left)
      rightOf.set(id, nd.right)
      childrenMap.set(id, [nd.left, nd.right].filter((x): x is number => x != null))
    })
  }
  const childrenOfFn = (id: number) => childrenMap.get(id) ?? []
  const order = traverseOrder(root, childrenOfFn, mode, {
    leftOf: (id) => leftOf.get(id) ?? null,
    rightOf: (id) => rightOf.get(id) ?? null
  })

  /** 当前树帧：doneIds 累计已访问，actId 当前访问（bin 传左右槽位，单侧孩子不跑偏） */
  const frame = (doneIds: number[], actId: number | undefined, narration: string, note?: string) => {
    const nodes: TNode[] = []
    const draw = (id: number) => {
      nodes.push(bin(id, keys[id], leftOf.get(id) ?? null, rightOf.get(id) ?? null, {
        keyActive: actId === id ? [0] : undefined,
        keyHighlight: doneIds.includes(id) ? [0] : undefined,
        nodeLabel: actId === id ? '当前' : (doneIds.includes(id) ? '已访问' : undefined)
      }))
      for (const k of childrenOfFn(id)) draw(k)
    }
    if (root != null) draw(root)
    steps.push(tstep(nodes, root, narration, note))
  }

  const modeName = MODE_NAME[mode]
  const total = order.length
  frame([],
    `带空标记先序建树完成（# 即空子树，空标记同样要消费，兄弟子树位置才不会错位）；本动画演 ${modeName}遍历——${mode === 'level' ? '先被访问的结点其孩子也先被访问，把栈换成队列，输出天然一层一层' : '递归三版只差访问根的时机'}`,
    '7 种遍历：先/中/后序各一个递归版 + 一个非递归栈版 + 层序队列版，递归与非递归对拍验证')

  const visited: number[] = []
  order.forEach((id, k) => {
    const kids = childrenOfFn(id)
    const l = kids[0] !== undefined ? keys[kids[0]] : null
    const r = kids[1] !== undefined ? keys[kids[1]] : null
    const pos = `第 ${k + 1}/${total} 个`
    let nar: string
    if (mode === 'pre') {
      nar = `${modeName}访问 ${keys[id]}（${pos}）：第一次路过即输出；随后递归左子树${l ? `（进入 ${l}）` : '（空，返回）'}、再右子树${r ? `（进入 ${r}）` : '（空，返回）'}`
    } else if (mode === 'in') {
      nar = `${modeName}访问 ${keys[id]}（${pos}）：左子树${l ? `（${l} 侧）已归来` : '为空已归来'}，轮到根输出；随后转入右子树${r ? `（进入 ${r}）` : '（空，返回）'}`
    } else if (mode === 'post') {
      nar = `${modeName}访问 ${keys[id]}（${pos}）：左右子树都已归来（左${l ?? '空'}、右${r ?? '空'}），才输出自己`
    } else {
      nar = kids.length
        ? `${modeName}访问 ${keys[id]}（${pos}）：出队即访问，其孩子 ${kids.map(c => keys[c]).join('、')} 依次入队——先访问者其孩子先被访问`
        : `${modeName}访问 ${keys[id]}（${pos}）：出队即访问（叶子，无孩子入队）`
    }
    visited.push(id)
    frame([...visited], id, nar,
      k === 0 ? `${modeName}序第一个：${mode === 'pre' ? '根最先' : mode === 'in' ? '最左结点最先' : mode === 'post' ? '最深的叶子最先' : '根先出队'}` : undefined)
  })

  const seq = order.map(id => keys[id]).join(' ')
  frame([...visited], undefined,
    `完成：${modeName}序 = ${seq}${mode === 'in' ? '（若这是 BST，中序即升序）' : ''}——递归 O(h) 系统栈、非递归 O(h) 显式栈、层序 O(w) 队列`,
    '非递归三版共用骨架：非空压栈向左、空了弹栈向右；后序多记一个 last_visited 防右子树反复下钻')

  return steps
}
