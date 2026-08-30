// docs/.vitepress/theme/visualizer/steps/treeKit.ts
// 批3 Tree 渲染器共用小工具（state 契约见 renderers/Tree.vue 文件头 JSDoc）。
// 全部纯函数：不改入参、输出确定。
import type { Step } from '../types'
import type { TNode, TreeState } from './states'

export type { TNode, TreeState }

/** 组一帧 Tree state（Tree 渲染器不走 Step.active/highlights 通道，键位/边高亮在 state 内） */
export function tstep(nodes: TNode[], rootId: number | null, narration: string, note?: string): Step {
  return { state: { nodes, rootId, note } as TreeState, highlights: [], active: null, narration }
}

/** 二叉结点便捷构造（keys 长度 1；left/right 可空，自动生成 L/R 槽位 sides） */
export function bin(
  id: number, key: number | string, left?: number | null, right?: number | null,
  extra?: Partial<TNode>
): TNode {
  const children: number[] = []
  const sides: ('L' | 'R')[] = []
  if (left != null) { children.push(left); sides.push('L') }
  if (right != null) { children.push(right); sides.push('R') }
  return { id, keys: [key], children, sides, ...extra }
}

/** 一般树结点（1 键 + 任意孩子） */
export function mnode(
  id: number, key: number | string, children: number[], extra?: Partial<TNode>
): TNode {
  return { id, keys: [key], children, ...extra }
}

/** 虚拟总根（0 键 = 外部/虚拟结点，虚线框）：森林/并查集森林等"多根"场景的汇总结点 */
export function vroot(id: number, children: number[], label?: string, extra?: Partial<TNode>): TNode {
  return { id, keys: [], children, nodeLabel: label, ...extra }
}

/** 层序建完全树（06_树/01、02、05 的固定父子约定）：第 i 个（0 起）孩子为第 2i+1、2i+2 个 */
export function levelComplete<T>(values: T[]): { childrenOf: Map<number, number[]>; parentOf: Map<number, number> } {
  const childrenOf = new Map<number, number[]>()
  const parentOf = new Map<number, number>()
  for (let i = 0; i < values.length; i++) {
    const kids: number[] = []
    if (2 * i + 1 < values.length) { kids.push(2 * i + 1); parentOf.set(2 * i + 1, i) }
    if (2 * i + 2 < values.length) { kids.push(2 * i + 2); parentOf.set(2 * i + 2, i) }
    childrenOf.set(i, kids)
  }
  return { childrenOf, parentOf }
}

export interface BinTree {
  root: number | null
  /** id -> {key, left, right}；id 由调用方给的序号函数决定 */
  nodes: Map<number, { key: string; left: number | null; right: number | null }>
}

/**
 * 带空标记先序建树（对应 bitree.c bitree_create / thread_tree.c tt_create，
 * 源码空标记为 -1，动画输入用 '#'；空标记同样要消费，兄弟子树位置才不会错位）。
 * 返回 null 表示标记串与树形不一致（validate 据此报错）。
 */
export function preNullBuild(tokens: string[]): BinTree | null {
  const nodes = new Map<number, { key: string; left: number | null; right: number | null }>()
  let pos = 0
  let nextId = 0
  const build = (): number | null => {
    if (pos >= tokens.length) return null // 串提前耗尽：交由末尾校验判错
    const t = tokens[pos++]
    if (t === '#') return null
    const id = nextId++
    const node = { key: t, left: build(), right: build() }
    nodes.set(id, node)
    return id
  }
  const root = build()
  if (pos !== tokens.length) return null // 有剩余标记：结构未对齐
  return { root, nodes }
}

/** 多叉树 / 二叉树共用：先/中/后/层序。中序需要左右槽位语义，传 lr（缺省取 children[0]/[1]，仅当两孩子齐全时正确） */
export function traverseOrder(
  root: number | null,
  childrenOf: (id: number) => number[],
  mode: 'pre' | 'in' | 'post' | 'level',
  lr?: { leftOf: (id: number) => number | null; rightOf: (id: number) => number | null }
): number[] {
  const out: number[] = []
  if (root == null) return out
  if (mode === 'level') {
    const q = [root]
    while (q.length) {
      const cur = q.shift() as number
      out.push(cur)
      q.push(...childrenOf(cur))
    }
    return out
  }
  if (mode === 'pre') {
    const rec = (id: number) => { out.push(id); for (const k of childrenOf(id)) rec(k) }
    rec(root)
  } else if (mode === 'in') {
    const left = lr?.leftOf ?? ((id: number) => childrenOf(id)[0] ?? null)
    const right = lr?.rightOf ?? ((id: number) => childrenOf(id)[1] ?? null)
    const rec = (id: number) => {
      const l = left(id)
      if (l != null) rec(l)
      out.push(id)
      const r = right(id)
      if (r != null) rec(r)
    }
    rec(root)
  } else {
    const rec = (id: number) => { for (const k of childrenOf(id)) rec(k); out.push(id) }
    rec(root)
  }
  return out
}
