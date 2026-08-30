// docs/.vitepress/theme/visualizer/steps/huffmanBuild.ts
import type { Step } from '../types'
import { tstep, mnode, vroot, type TNode } from './treeKit'

// 哈夫曼树构造（对应 06_树/06_哈夫曼树/huffman_tree.c ht_build + ht_wpl）：
//   静态三叉链表 nodes[1..2n-1]：前 n 格叶子（输入序），后 n-1 格合并产生；
//   select_two_min 线性扫"无双亲且未被选"的最小两结点（s1 权 ≤ s2 权，平手取下标小者），
//   小者为左（left=s1, right=s2），新结点权 = 二者之和；共合并 n-1 轮。
//   WPL = Σ w_i × l_i（叶深），哈夫曼树使 WPL 最小，恰等于反复合并两最小权的合并总代价。
// 输入权值串（2~8 个正整数）。
export interface HuffmanBuildInput { weights: number[] }

export function huffmanBuildSteps(input: HuffmanBuildInput): Step[] {
  const steps: Step[] = []
  const ws = input.weights
  const n = ws.length

  // 静态数组模拟：下标 1..2n-1，0 表空（与源码一致）
  const weight: number[] = [0, ...ws]
  const parent: number[] = Array(2 * n).fill(0)
  const left: number[] = Array(2 * n).fill(0)
  const right: number[] = Array(2 * n).fill(0)

  /** 源码同款 select_two_min：range 内 parent==0 的最小两个（下标小者优先） */
  const selectTwoMin = (range: number): [number, number] => {
    let s1 = 0, s2 = 0, min1 = 0x7FFFFFFF, min2 = 0x7FFFFFFF
    for (let i = 1; i <= range; i++) {
      if (parent[i] !== 0) continue
      if (weight[i] < min1) { min2 = min1; s2 = s1; min1 = weight[i]; s1 = i }
      else if (weight[i] < min2) { min2 = weight[i]; s2 = i }
    }
    return [s1, s2]
  }

  const liveRoots = (): number[] => {
    const out: number[] = []
    for (let i = 1; i <= 2 * n - 1; i++) if (weight[i] !== 0 && parent[i] === 0) out.push(i)
    return out
  }

  /** 森林/树帧。asForest=true 且根数>1 时挂虚拟总根；doneLeafIdx 高亮当前累计的 WPL 叶子 */
  const frame = (narration: string, note?: string, activeIds: number[] = [], doneLeafIdx: number[] = [], asForest = false) => {
    const nodes: TNode[] = []
    const draw = (id: number) => {
      const kids = [left[id], right[id]].filter(c => c !== 0)
      nodes.push(mnode(id, weight[id], kids, {
        nodeLabel: `w=${weight[id]}`,
        keyActive: activeIds.includes(id) ? [0] : undefined,
        keyHighlight: doneLeafIdx.includes(id) ? [0] : undefined
      }))
      for (const c of kids) draw(c)
    }
    const roots = liveRoots()
    if (asForest && roots.length > 1) {
      nodes.unshift(vroot(0, roots, `森林（${roots.length} 棵）`))
      for (const r of roots) draw(r)
      steps.push(tstep(nodes, 0, narration, note))
    } else {
      for (const r of roots) draw(r)
      steps.push(tstep(nodes, roots[0] ?? null, narration, note))
    }
  }

  // 0. 开场：n 个叶子的森林
  frame(
    `哈夫曼构造开始：${n} 个权值 ${ws.join(',')} 各自成树（静态数组 nodes[1..${2 * n - 1}]，前 ${n} 格叶子按输入序）`,
    '构造贪心：每轮取"双亲为空且权最小"的两结点合并，新结点权 = 二者之和；共合并 n-1 轮',
    [], [], true)

  // 1. n-1 轮合并
  for (let i = n + 1; i <= 2 * n - 1; i++) {
    const [s1, s2] = selectTwoMin(i - 1)
    frame(
      `第 ${i - n} 轮选最小：无双亲结点中权最小的两个是 w=${weight[s1]}（下标 ${s1}）与 w=${weight[s2]}（下标 ${s2}）——平手取下标小者，保证确定性`,
      undefined, [s1, s2], [], true)
    parent[s1] = i
    parent[s2] = i
    left[i] = s1  // 小者为左
    right[i] = s2
    weight[i] = weight[s1] + weight[s2]
    const remaining = liveRoots().length
    frame(
      `新结点 nodes[${i}]：权 = ${weight[s1]} + ${weight[s2]} = ${weight[i]}，left=小者 ${s1}、right=${s2}；两棵子树出列，森林剩 ${remaining} 棵`,
      undefined, [i], [], remaining > 1)
  }

  // 2. WPL 逐叶累加
  const depthOf = (leaf: number): number => {
    let d = 0
    let p = parent[leaf]
    while (p !== 0) { d++; p = parent[p] }
    return d
  }
  const terms: string[] = []
  let acc = 0
  const leafIds = Array.from({ length: n }, (_, k) => k + 1)
  leafIds.forEach((leaf, k) => {
    const d = depthOf(leaf)
    const term = weight[leaf] * d
    acc += term
    terms.push(`${weight[leaf]}×${d}`)
    frame(
      `WPL 累加（${k + 1}/${n}）：叶 w=${weight[leaf]}（下标 ${leaf}）深度 ${d}——${weight[leaf]}×${d}=${term}，累计 WPL=${acc}`,
      undefined, [leaf], leafIds.slice(0, k + 1), false)
  })
  frame(
    `WPL = ${terms.join(' + ')} = ${acc}——哈夫曼树使 WPL 最小，其值恰等于"反复合并两个最小权"的合并总代价（合并果子）`,
    '编码：从叶子向根回溯记转向位再反转（左 0 右 1），任意编码均非其他编码的前缀（前缀码）',
    [], leafIds, false)

  return steps
}
