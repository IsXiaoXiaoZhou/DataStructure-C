// docs/.vitepress/theme/visualizer/steps/seqBitreeIndex.ts
import type { Step } from '../types'
import { tstep, levelComplete, bin } from './treeKit'

// 二叉树顺序存储（对应 06_树/02_二叉树顺序存储/seq_binary_tree.c）：
//   data[1..last] 按完全二叉树编号连续存放，0 号弃用；
//   编号 i 的双亲 ⌊i/2⌋（sbt_parent，整数除法自动下取整）、左孩子 2i、右孩子 2i+1。
// 输入为层序数据（≤7 个），完全树形态保证编号关系恒成立；边标 2i / 2i+1，结点标 i。
export interface SeqBitreeInput { values: number[] }

export function seqBitreeIndexSteps(input: SeqBitreeInput): Step[] {
  const steps: Step[] = []
  const v = input.values
  const n = v.length
  const num = (i: number) => i + 1 // 0 起下标 → 1 起编号
  const { childrenOf, parentOf } = levelComplete(v)

  // 边标签（2i / 2i+1）挂在孩子结点的 edgeLabel 上，一次算好
  const edgeLabelOf = new Map<number, string>()
  for (let i = 0; i < n; i++) {
    const kids = childrenOf.get(i) ?? []
    if (kids[0] !== undefined) edgeLabelOf.set(kids[0], `2×${num(i)}=${2 * num(i)}`)
    if (kids[1] !== undefined) edgeLabelOf.set(kids[1], `2×${num(i)}+1=${2 * num(i) + 1}`)
  }

  /** 基础帧：全树 + 每结点 i 标签 + 固定边标签；actNode 点名、doneNodes 累计、hotChild 高亮其双亲边
   *  用 bin（左右槽位 sides）保证"只有左孩子/只有右孩子"时孩子画在正确一侧 */
  const frame = (labels: Record<number, string>, narration: string, note?: string,
                 actNode?: number, doneNodes: number[] = [], hotChild?: number) =>
    steps.push(tstep(
      v.map((val, i) => {
        const kids = childrenOf.get(i) ?? []
        const l = kids[0] !== undefined ? v[kids[0]] : null
        const r = kids[1] !== undefined ? v[kids[1]] : null
        return bin(val, val, l, r, {
          nodeLabel: labels[i],
          edgeLabel: edgeLabelOf.get(i),
          keyActive: actNode === i ? [0] : undefined,
          keyHighlight: doneNodes.includes(i) ? [0] : undefined,
          edgeActive: hotChild !== undefined && parentOf.has(hotChild) && parentOf.get(hotChild) === i
            ? [(kids as number[]).indexOf(hotChild)]
            : undefined
        })
      }),
      v[0], narration, note
    ))

  const base: Record<number, string> = {}
  v.forEach((_, i) => { base[i] = `i=${num(i)}` })

  // 0. 开场
  frame(base,
    `完全二叉树层序 ${v.join(',')}：data[1..${n}] 连续存放（0 号弃用），编号 i 的左孩子 2i、右孩子 2i+1、双亲 ⌊i/2⌋——三个关系式无 +1 偏移，与教材编号一致`,
    '顺序存储天然适合完全二叉树（无空间浪费）；深度 k 的一般二叉树按最坏形态补位将浪费 2^k − n 个单元')

  // 1. 逐结点指认三种关系
  for (let i = 0; i < n; i++) {
    const labels: Record<number, string> = { ...base }
    if (parentOf.has(i)) {
      const p = parentOf.get(i)!
      labels[i] = `i=${num(i)} 双亲=${num(p)}`
      frame(labels,
        `编号 ${num(i)} 的双亲 = ⌊${num(i)}/2⌋ = ${num(p)}（sbt_parent：整数除法自动下取整）——双亲/孩子定位 O(1)，顺序存储的核心优势`,
        undefined, i, [], i)
    } else {
      frame(labels,
        `编号 1 是根：sbt_parent 对它返回 DS_ERROR"根结点无双亲"——根没有 ⌊i/2⌋ 可讲，其余每个编号都有`,
        undefined, i)
    }
    const kids = childrenOf.get(i) ?? []
    if (kids[0] !== undefined) {
      const l = kids[0]
      labels[l] = `i=${num(l)} =2×${num(i)}`
      frame(labels,
        `左孩子：2i = 2×${num(i)} = ${2 * num(i)}，存在——data[${2 * num(i)}] = ${v[l]}（sbt_left_child）`,
        undefined, l, [], l)
    } else {
      frame(labels,
        `左孩子：2i = ${2 * num(i)} > last=${n}，无左孩子（sbt_left_child 返回 DS_ERROR）`,
        undefined, i)
    }
    if (kids[1] !== undefined) {
      const r = kids[1]
      labels[r] = `i=${num(r)} =2×${num(i)}+1`
      frame(labels,
        `右孩子：2i+1 = 2×${num(i)}+1 = ${2 * num(i) + 1}，存在——data[${2 * num(i) + 1}] = ${v[r]}（sbt_right_child）`,
        undefined, r, [], r)
    } else {
      frame(labels,
        `右孩子：2i+1 = ${2 * num(i) + 1} > last=${n}，无右孩子（sbt_right_child 返回 DS_ERROR）——完全树只在最后一层缺孩子`,
        undefined, i)
    }
  }

  // 2. 收尾
  frame(base,
    `编号关系盘点完毕：${n} 个结点占满 data[1..${n}] 无空洞；层序连续追加保持完全形态，编号关系恒成立`,
    '遍历也可按编号递推：左 2i、右 2i+1、双亲 ⌊i/2⌋（sbt_preorder/inorder/postorder 均基于此）')

  return steps
}
