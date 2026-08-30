// docs/.vitepress/theme/visualizer/steps/treeStorageViews.ts
import type { Step } from '../types'
import { tstep, levelComplete, mnode, bin, type TNode } from './treeKit'

// 树的三种存储结构（对应 06_树/01_树的存储结构/tree_storage.c）：
//   双亲表示  pt_*：nodes[i] = {data, parent 下标}，根 parent=-1；找双亲 O(1)，找孩子全表扫描 O(n)
//   孩子链表  clt_*：数组每格挂孩子链（ChildBox 无尾指针，尾插保下标序）；找孩子 O(d)，找双亲扫全部链
//   孩子兄弟  cst_*：first_child/next_sibling 二叉链表，树转二叉树的钥匙；n 结点剩 n+1 个空链域
// 输入为层序数据（≤5 个），按完全树逐层挂父子：第 i 个（0 起）孩子是第 2i+1、2i+2 个。
// 画面主体固定为孩子兄弟二叉形态（任务约定），双亲/孩子链信息用 nodeLabel + note 辅助表达。
export interface StorageViewsInput { values: number[] }

export function treeStorageViewsSteps(input: StorageViewsInput): Step[] {
  const steps: Step[] = []
  const v = input.values
  const n = v.length
  const { childrenOf, parentOf } = levelComplete(v)
  const kidsOf = (i: number) => (childrenOf.get(i) ?? []).map(k => v[k])

  // 孩子兄弟二叉形态（cst_from_parent：第一遍建结点，第二遍按双亲挂链——尾插保兄弟序=下标序）
  // CS 指针：left = 第一个孩子（first_child），right = 下一兄弟（next_sibling，同双亲孩子序）
  const csLeft = new Map<number, number>()
  const csRight = new Map<number, number>()
  for (let i = 0; i < n; i++) {
    const kids = childrenOf.get(i) ?? []
    if (kids.length) csLeft.set(i, kids[0])
    const sibs = parentOf.has(i) ? (childrenOf.get(parentOf.get(i)!) ?? []) : []
    const si = sibs.indexOf(i)
    if (si >= 0 && si + 1 < sibs.length) csRight.set(i, sibs[si + 1])
  }

  /** 孩子兄弟二叉形态帧（nodeLabel 覆盖式提供；bin 自动生成 L/R 槽位，单侧兄弟不跑偏） */
  const csNodes = (labels: Record<number, string>, act: number[] = [], done: number[] = []): { nodes: TNode[]; rootId: number } => {
    const nodes: TNode[] = v.map((val, i) => bin(val, val,
      csLeft.has(i) ? v[csLeft.get(i)!] : null,
      csRight.has(i) ? v[csRight.get(i)!] : null,
      {
        nodeLabel: labels[i],
        keyActive: act.includes(i) ? [0] : undefined,
        keyHighlight: done.includes(i) ? [0] : undefined
      }))
    return { nodes, rootId: v[0] }
  }
  const push = (labels: Record<number, string>, narration: string, note?: string, act: number[] = [], done: number[] = []) => {
    const { nodes, rootId } = csNodes(labels, act, done)
    steps.push(tstep(nodes, rootId, narration, note))
  }
  const defaultLabels = (): Record<number, string> => ({})

  // 0. 开场：先立住这棵树（多叉形态一帧，仅开头用）
  steps.push(tstep(
    v.map((val, i) => mnode(val, val, (childrenOf.get(i) ?? []).map(k => v[k]))), v[0],
    `这棵 ${n} 结点的树（层序 ${v.join(',')}）将用三种方式各存一遍：双亲表示、孩子链表、孩子兄弟——没有谁最好，只有对哪类操作便宜`,
    '画面主体统一为孩子兄弟二叉形态（树转二叉树的钥匙），另两种表示用标签与说明表达'
  ))

  // 1. 双亲表示（pt_*）
  const pLabels: Record<number, string> = {}
  v.forEach((_, i) => { pLabels[i] = `parent=${parentOf.has(i) ? parentOf.get(i) : -1}` })
  push(pLabels,
    `双亲表示：数组每格存 data 加 parent 下标，根的 parent 为 -1（结点 0 起编号，与 pt_insert 的数组下标一致）`,
    '双亲表示法：nodes[i] = {data, parent}，找双亲 O(1)，找孩子要全表扫描 O(n)',
    [], [0])
  if (n >= 2) {
    const target = 1
    const childIdx = childrenOf.get(target) ?? []
    const cLabels: Record<number, string> = { ...pLabels }
    for (const c of childIdx) cLabels[c] = `p=${target}✓`
    push(cLabels,
      `pt_children(t, ${target}) 找孩子：双亲表示必须全表扫描——把 nodes[0..${n - 1}] 从头滤到尾看谁的 parent 等于 ${target}，${n} 格扫完命中 ${childIdx.length} 个孩子，O(n)`,
      undefined, [target], childIdx)
    push(pLabels,
      `反过来找双亲只要一跳：nodes[${target}].parent = ${parentOf.get(target)} 直接给出下标，O(1)——它把"向上"这条路修到极致，07 模块并查集就是它燃到最后的形态`,
      undefined, [target])
  }

  // 2. 孩子链表（clt_*）
  const cLabels2: Record<number, string> = {}
  v.forEach((_, i) => {
    const kids = kidsOf(i)
    cLabels2[i] = kids.length ? `孩子链 ${kids.join('→')}` : '孩子链 ∅'
  })
  push(cLabels2,
    `孩子链表：数组每格挂一条单链串全部孩子的下标（ChildBox 无尾指针，插入沿链走到尾再接上，孩子序=下标序）`,
    '孩子表示法：找孩子沿链 O(d)，找双亲要扫遍所有链 O(n)', [], [0])
  if (n >= 2) {
    const target = 0
    const childIdx = childrenOf.get(target) ?? []
    push(cLabels2,
      `clt_children(t, ${target}) 找孩子：顺链走 ${childIdx.length} 步就完，O(d)——找孩子是它的主场`,
      undefined, [target], childIdx)
    push(cLabels2,
      `但找双亲要扫遍所有链：想知道 ${v[1]} 的双亲是谁，得把 ${n} 条孩子链都翻一遍，O(n)`,
      undefined, [], [1])
  }

  // 3. 孩子兄弟（cst_*）
  const sLabels: Record<number, string> = {}
  v.forEach((_, i) => {
    const fc = csLeft.has(i) ? v[csLeft.get(i)!] : '-'
    const ns = csRight.has(i) ? v[csRight.get(i)!] : '-'
    sLabels[i] = `fc→${fc} ns→${ns}`
  })
  push(sLabels,
    `孩子兄弟表示：每结点两个指针——first_child 指第一个孩子（画为左），next_sibling 指下一兄弟（画为右）；孩子再多也装得下，兄弟们自己手拉手排成一串`,
    '孩子兄弟表示法：多叉树就此变成规规矩矩的二叉树，树/森林与二叉树互转的桥梁',
    [], [0])
  const nullCount = v.length + 1 // n 结点 2n 指针域，n-1 条边，剩 n+1 个空链域
  push(sLabels,
    `数一数空链域：${n} 个结点 ${2 * n} 个指针域，${n - 1} 条边用掉 ${n - 1} 个，剩 n+1 = ${nullCount} 个空指针——04 模块线索化全靠这批"废指针"`,
    undefined, [], Array.from({ length: n }, (_, i) => i))

  // 4. 收尾：三种表示的代价对照
  const costNote = '找双亲：双亲表示 O(1) / 孩子链表 O(n) / 孩子兄弟 O(n)；找孩子：双亲表示 O(n) / 孩子链表 O(d) / 孩子兄弟 O(d)'
  push(defaultLabels(),
    `三种表示对同一棵树信息等价（三方对拍的前提是结点下标一致），选谁看操作频率：频繁找双亲选双亲表示，频繁处理孩子选孩子链表，要转二叉树或做遍历选孩子兄弟`,
    costNote)

  return steps
}
