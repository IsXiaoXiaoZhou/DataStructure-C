// docs/.vitepress/theme/visualizer/steps/bstInsertSearch.ts
import type { Step } from '../types'
import { tstep, bin, type TNode } from './treeKit'

// 二叉排序树插入与查找（对应 08_查找/04_二叉排序树/bst.c）：
//   插入 bst_insert：沿比较路径递归到底，挂新叶子——新结点必为叶子；重复报 DS_ERROR。
//   查找 bst_search：while 循环逐结点比较，小于走左、大于走右；走空报 DS_NOT_FOUND。
// 输入 "插入串|目标"（如 "5,2,9,1,7|7"）：先逐个插入，再查找目标。
export interface BstInput { keys: number[]; target: number }

interface BNode { key: number; l: number | null; r: number | null }

export function bstInsertSearchSteps(input: BstInput): Step[] {
  const steps: Step[] = []
  let root: number | null = null
  const nodes = new Map<number, BNode>()
  const kidsOf = (id: number): number[] => {
    const nd = nodes.get(id)!
    return [nd.l, nd.r].filter((x): x is number => x != null)
  }

  /** 全树帧；path = 根→当前比较结点路径（逐段 edgeActive），act 当前结点，done 高亮（命中） */
  const frame = (narration: string, note?: string, path: number[] = [], act?: number, done: number[] = []) => {
    const out: TNode[] = []
    const draw = (id: number): void => {
      const nd = nodes.get(id)!
      const kids = kidsOf(id)
      const pi = path.indexOf(id)
      out.push(bin(id, nd.key, nd.l, nd.r, {
        keyActive: act === id ? [0] : undefined,
        keyHighlight: done.includes(id) ? [0] : undefined,
        nodeLabel: act === id ? '当前比较' : undefined,
        edgeActive: pi >= 0 && pi + 1 < path.length && kids.includes(path[pi + 1])
          ? [kids.indexOf(path[pi + 1])]
          : undefined
      }))
      for (const k of kids) draw(k)
    }
    if (root != null) draw(root)
    steps.push(tstep(out, root, narration, note))
  }

  // 1. 插入阶段
  frame('空树：从第一个关键字开始，按 BST 规则"小左大右"逐个下落插入',
    'BST 性质：左子树所有结点 < 根 < 右子树所有结点（子树递归成立）；中序遍历 = 升序序列')
  input.keys.forEach((key, ki) => {
    frame(`插入 ${key}（第 ${ki + 1}/${input.keys.length} 个）：从根开始比较下落`, undefined, root != null ? [root] : undefined, root ?? undefined)
    if (root == null) {
      nodes.set(key, { key, l: null, r: null })
      root = key
      frame(`空树：${key} 直接成为根（bst_insert 命中 *root == NULL 分支）`, undefined, [], key)
      return
    }
    let cur = root
    for (;;) {
      const nd = nodes.get(cur)!
      if (key === nd.key) {
        frame(`key = ${cur}：关键字已存在，不允许重复插入（DS_ERROR）——BST 靠查找定位，重复键无处安放`,
          undefined, pathTo(root, cur), cur)
        return
      }
      const goLeft = key < nd.key
      const next = goLeft ? nd.l : nd.r
      frame(`${key} vs ${cur}：${key} ${goLeft ? '<' : '>'} ${cur} → 递归${goLeft ? '左' : '右'}子树`,
        undefined, pathTo(root, cur), cur)
      if (next == null) {
        nodes.set(key, { key, l: null, r: null })
        if (goLeft) nodes.get(cur)!.l = key
        else nodes.get(cur)!.r = key
        frame(`走到空指针：挂新叶子——${cur} 的${goLeft ? '左' : '右'}孩子 = ${key}（新结点必为叶子，这是 BST 插入的形态保证）`,
          undefined, pathTo(root, cur), key)
        return
      }
      cur = next
    }
  })

  // 2. 查找阶段
  const target = input.target
  frame(`插入完毕（${input.keys.length} 结点）。查找阶段：bst_search(${target})，while 循环逐结点比较`,
    '查找与折半类似，但树形由插入顺序决定——同一组键不同插入序，查找路径可以差很远', root != null ? [root] : undefined, root ?? undefined)
  if (root == null) {
    frame('树为空：DS_NOT_FOUND', undefined)
    return steps
  }
  let cur: number | null = root
  let compares = 0
  const path: number[] = [root]
  while (cur != null) {
    compares++
    const nd = nodes.get(cur)!
    if (target === nd.key) {
      frame(`${target} = ${cur}：查找成功！比较 ${compares} 次命中（bst_search 返回结点指针）`,
        undefined, path, cur, [cur])
      frame(`演示结束：目标 ${target} 在树中，比较 ${compares} 次；中序遍历 = 升序序列 ${inorderKeys()}`,
        '同一组键，插入顺序决定树形与查找效率——最坏退化成链 O(n)，平衡化见 05 模块', path, undefined, [cur])
      return steps
    }
    const goLeft = target < nd.key
    frame(`${target} vs ${cur}：${target} ${goLeft ? '<' : '>'} ${cur} → 走${goLeft ? '左' : '右'}子树`,
      undefined, path, cur)
    const next = goLeft ? nd.l : nd.r
    if (next == null) {
      frame(`走到空指针：DS_NOT_FOUND 未找到目标关键字（比较 ${compares} 次）——业务正常结果，非异常`,
        undefined, path)
      return steps
    }
    cur = next
    path.push(cur)
  }
  return steps

  // ---- 局部工具 ----
  function pathTo(r: number, id: number): number[] {
    const out: number[] = []
    let cur: number | null = r
    while (cur != null) {
      out.push(cur)
      if (cur === id) break
      cur = id > cur ? nodes.get(cur)!.r : nodes.get(cur)!.l // id 即键，按 BST 性质走侧
    }
    return out
  }
  function inorderKeys(): string {
    const out: number[] = []
    const rec = (id: number | null) => {
      if (id == null) return
      rec(nodes.get(id)!.l)
      out.push(id)
      rec(nodes.get(id)!.r)
    }
    rec(root)
    return out.join(', ')
  }
}
