// docs/.vitepress/theme/visualizer/steps/forestConvert.ts
import type { Step } from '../types'
import { tstep, mnode, bin, vroot, type TNode } from './treeKit'

// 森林 → 二叉树（对应 06_树/05_森林与二叉树转换/forest_bitree.c forest_to_bitree）：
//   规则（教材标准）：树内 first_child → 左孩子、next_sibling → 右孩子；
//   树间第 i+1 棵树根挂第 i 棵树根的右链（next_sibling → right）。
//   性质：森林先序 == 转换后二叉树先序；森林后序 == 转换后二叉树中序。
// 输入 "1,2,3;4,5"：分号分段，每段 = 一棵树的层序数据（段内按完全树挂父子）。
export interface ForestConvertInput { trees: number[][] }

export function forestConvertSteps(input: ForestConvertInput): Step[] {
  const steps: Step[] = []
  const trees = input.trees

  // 每棵树：层序 → 完全树父子；全局 id 直接用数值本身（validate 保证森林内唯一）
  interface Seg { root: number; vals: number[]; kids: Map<number, number[]> }
  const segs: Seg[] = trees.map(vals => {
    const kids = new Map<number, number[]>()
    vals.forEach((v, i) => kids.set(v, [2 * i + 1, 2 * i + 2].filter(c => c < vals.length).map(c => vals[c])))
    return { root: vals[0], vals, kids }
  })
  const segOf = new Map<number, Seg>()
  for (const s of segs) for (const v of s.vals) segOf.set(v, s)

  const roots = segs.map(s => s.root)

  // 转换后二叉形态（复制重建，不共享结点）：left=first_child，right=next_sibling；根链挂右
  const binLeft = new Map<number, number>()
  const binRight = new Map<number, number>()
  for (const s of segs) {
    for (const v of s.vals) {
      const kids = s.kids.get(v) ?? []
      if (kids.length) binLeft.set(v, kids[0])
      for (let i = 1; i < kids.length; i++) binRight.set(kids[i - 1], kids[i])
    }
  }
  for (let i = 1; i < roots.length; i++) binRight.set(roots[i - 1], roots[i])
  const binChildren = (v: number): number[] => [
    ...(binLeft.has(v) ? [binLeft.get(v)!] : []),
    ...(binRight.has(v) ? [binRight.get(v)!] : [])
  ]

  /** 森林帧（多叉形态，虚拟总根 keys=[]） */
  const forestFrame = (narration: string, note?: string, hotEdge?: { p: number; c: number }) =>
    steps.push(tstep(
      [
        vroot(0, roots, '森林（根链）'),
        ...segs.flatMap(s => s.vals.map(v => mnode(v, v, s.kids.get(v) ?? [], {
          edgeActive: hotEdge && hotEdge.p === v && (s.kids.get(v) ?? []).includes(hotEdge.c)
            ? (s.kids.get(v) ?? []).indexOf(hotEdge.c)
            : undefined
        })))
      ],
      0, narration, note
    ))

  /** 二叉形态帧；hotEdge = (父, 孩子) 高亮该条连线 */
  const binFrame = (narration: string, note?: string, hotEdge?: { p: number; c: number }) => {
    const out: TNode[] = []
    const draw = (v: number, seen: Set<number>) => {
      if (seen.has(v)) return
      seen.add(v)
      const kids = binChildren(v)
      out.push(bin(v, v, binLeft.get(v) ?? null, binRight.get(v) ?? null, {
        edgeActive: hotEdge && hotEdge.p === v && kids.includes(hotEdge.c)
          ? [kids.indexOf(hotEdge.c)]
          : undefined,
        nodeLabel: hotEdge && hotEdge.p === v ? '正在重连' : undefined
      }))
      for (const c of kids) draw(c, seen)
    }
    draw(roots[0], new Set())
    steps.push(tstep(out, roots[0], narration, note))
  }

  // 0. 原森林
  forestFrame(
    `原森林：${segs.length} 棵树（${trees.map(t => t.join(',')).join(' ； ')}），根经 next_sibling 串成根链`,
    '转换规则：树内 first_child → 左孩子、next_sibling → 右孩子；树间第 i+1 棵树根挂第 i 棵树根的右链')

  // 1. 树内重连：每棵树逐结点
  for (const s of segs) {
    for (const v of s.vals) {
      const kids = s.kids.get(v) ?? []
      if (kids.length) {
        binFrame(
          `树内重连：${v} 的第一个孩子 ${kids[0]} 变左孩子（first_child → left），${v} 原本挂着 ${kids.length} 个孩子`,
          undefined, { p: v, c: kids[0] })
      }
      for (let i = 1; i < kids.length; i++) {
        binFrame(
          `树内重连：兄弟连线——${kids[i]} 变 ${kids[i - 1]} 的右孩子（next_sibling → right），兄弟们手拉手排成一串`,
          undefined, { p: kids[i - 1], c: kids[i] })
      }
    }
  }
  // 2. 树间重连：根链
  for (let i = 1; i < roots.length; i++) {
    binFrame(
      `树间重连：第 ${i + 1} 棵树根 ${roots[i]} 挂到第 ${i} 棵树根 ${roots[i - 1]} 的右链上（next_sibling → right）`,
      undefined, { p: roots[i - 1], c: roots[i] })
  }

  // 3. 转换后二叉形态
  binFrame(
    `转换完成：${roots[0]} 为二叉树根，整棵森林收进一棵二叉树（forest_to_bitree 采用复制重建，源与目标同时可读可销毁）`,
    '重要性质：森林先序遍历 == 转换后二叉树的先序遍历；森林后序遍历 == 转换后二叉树的中序遍历（roundtrip 对拍验证）')

  return steps
}
