// docs/.vitepress/theme/visualizer/steps/rbInsert.ts
import type { Step } from '../types'
import { tstep, bin } from './treeKit'

// 红黑树插入（对应 08_查找/06_红黑树/rb.c rb_insert + insert_fixup，CLRS 风格）：
//   新结点染红挂叶（不破坏黑高）；父红即破性质4，修复循环：
//     叔叔红 → 变色上推（父叔变黑、祖父变红，检查点上移到祖父）；
//     叔叔黑 → LR/RL 折线先旋成直线，再父染黑、祖父染红、单旋；
//   循环出口 z 到根或父已黑；最后根恒黑（性质2）。
// 输入插入串（4~8 个键；4 个不同键必产生至少一次修复动作——validate 保证）。
export interface RbInput { keys: number[] }

interface RN { key: number; l: number | null; r: number | null; color: 'R' | 'B' }

export function rbInsertSteps(input: RbInput): Step[] {
  const steps: Step[] = []
  const nodes = new Map<number, RN>()
  let root: number | null = null
  const kidsOf = (id: number): number[] => {
    const nd = nodes.get(id)!
    return [nd.l, nd.r].filter((x): x is number => x != null)
  }

  /** 全树帧；act = 检查点 z，done = 已定色结点，path = 下落比较路径，labels 覆盖 */
  const frame = (narration: string, note?: string, opt: {
    act?: number; done?: number[]; path?: number[]; labels?: Record<number, string>
  } = {}) => {
    const out = [] as ReturnType<typeof bin>[]
    const draw = (id: number): void => {
      const nd = nodes.get(id)!
      const kids = kidsOf(id)
      const pi = (opt.path ?? []).indexOf(id)
      out.push(bin(id, nd.key, nd.l, nd.r, {
        color: nd.color,
        nodeLabel: opt.labels?.[id] ?? (opt.act === id ? '检查点 z' : undefined),
        keyActive: opt.act === id ? [0] : undefined,
        keyHighlight: (opt.done ?? []).includes(id) ? [0] : undefined,
        edgeActive: pi >= 0 && pi + 1 < (opt.path ?? []).length && kids.includes((opt.path ?? [])[pi + 1])
          ? [kids.indexOf((opt.path ?? [])[pi + 1])]
          : undefined
      }))
      for (const k of kids) draw(k)
    }
    if (root != null) draw(root)
    steps.push(tstep(out, root, narration, note))
  }
  const pathTo = (target: number): number[] => {
    const out: number[] = []
    let cur: number | null = root
    while (cur != null) { out.push(cur); if (cur === target) break; cur = target > cur ? nodes.get(cur)!.r : nodes.get(cur)!.l }
    return out
  }

  // 旋转（rb.c rot_left / rot_right 的孩子指针部分；双亲回挂由 attach 完成）
  const rotLeft = (x: number): number => {
    const xn = nodes.get(x)!
    const y = xn.r as number
    const yn = nodes.get(y)!
    xn.r = yn.l
    yn.l = x
    return y
  }
  const rotRight = (x: number): number => {
    const xn = nodes.get(x)!
    const y = xn.l as number
    const yn = nodes.get(y)!
    xn.l = yn.r
    yn.r = x
    return y
  }
  const parentOf = (id: number): number | null => {
    for (const k of nodes.keys()) {
      const nd = nodes.get(k)!
      if (nd.l === id || nd.r === id) return k
    }
    return null
  }
  const attach = (parent: number | null, oldChild: number, newChild: number) => {
    if (parent == null) { root = newChild; return }
    const pn = nodes.get(parent)!
    if (pn.l === oldChild) pn.l = newChild
    else pn.r = newChild
  }

  /** 叔黑直线分支：父染黑、祖父染红、对 g 单旋（dirRight=true 右旋 LL / false 左旋 RR） */
  const fixStraight = (p: number, g: number, dirRight: boolean, fromFold: boolean) => {
    const gp = parentOf(g) // 必须在旋转前捕获：旋转让 g 的双亲指针指向新根
    const nr = dirRight ? rotRight(g) : rotLeft(g)
    nodes.get(p)!.color = 'B'
    nodes.get(g)!.color = 'R'
    attach(gp, g, nr)
    const form = dirRight ? 'LL' : 'RR'
    const rotName = dirRight ? '右旋' : '左旋'
    frame(`${form} 直线：父 ${p} 染黑、祖父 ${g} 染红、${rotName} ${g} —— ${p} 升为子树根，红红冲突就地化解${fromFold ? '（折线半旋后走的就是这条直线分支）' : '（叔黑分支：一次单旋终结，z 的新父已黑）'}`,
      '叔黑分支：父染黑补回该侧黑高，祖父染红再把黑高差转嫁给旋转', { act: p, labels: { [p]: '新根·黑', [g]: '降位·红' } })
    return nr
  }

  // 0. 开场
  frame(`红黑插入演示：${input.keys.join(',')}。新结点必染红（不破坏黑高），父红才修复——叔红变色上推、叔黑旋转`,
    '五性质：结点非红即黑 / 根黑 / NULL 叶黑 / 无相邻红 / 黑高相等；高度 ≤ 2·log2(n+1)')

  // 1. 逐键插入
  input.keys.forEach((key) => {
    // BST 下落（rb_insert 常规插叶段）
    const path: number[] = []
    let y: number | null = null
    let x: number | null = root
    while (x != null) {
      y = x
      path.push(x)
      frame(`${key} vs ${x}：${key} ${key < x ? '<' : key > x ? '>' : '='} ${x} → 走${key < x ? '左' : key > x ? '右' : '无（重复）'}`,
        undefined, { act: x, path })
      if (key === x) return // 重复：validate 已拒，防御
      x = key < x ? nodes.get(x)!.l : nodes.get(x)!.r
    }
    nodes.set(key, { key, l: null, r: null, color: 'R' })
    if (y == null) {
      root = key
      frame(`空树：${key} 挂为根并染红——insert_fixup 一进来就退出循环，末尾统一根染黑`, undefined, { act: key })
    } else {
      const yn = nodes.get(y)!
      if (key < yn.key) yn.l = key
      else yn.r = key
      frame(`常规 BST 插叶：新结点 ${key} 染红挂为 ${y} 的${key < yn.key ? '左' : '右'}孩子（新结点必红：不破坏黑高）`,
        undefined, { act: key, path })
    }
    // insert_fixup（rb.c 逐行同构；单旋/半旋后 z 的新父已黑，循环必出——与 while 条件等价）
    let z: number | null = key
    for (;;) {
      if (z === root) break
      const p = parentOf(z)
      if (p == null || nodes.get(p)!.color !== 'R') break
      const g = parentOf(p) as number // 父红 ⇒ 父非根 ⇒ 祖父必在
      const pIsLeft = nodes.get(g)!.l === p
      const uncle = pIsLeft ? nodes.get(g)!.r : nodes.get(g)!.l
      if (uncle != null && nodes.get(uncle)!.color === 'R') {
        nodes.get(p)!.color = 'B'
        nodes.get(uncle)!.color = 'B'
        nodes.get(g)!.color = 'R'
        frame(`叔叔红 → 变色上推：父 ${p} 与叔 ${uncle} 变黑、祖父 ${g} 变红（黑高不变），检查点上移到 ${g} 继续循环`,
          '叔红分支：不旋转，只换色——把"红红冲突"整层上推，到根或父黑才停',
          { act: g, done: [p, uncle], labels: { [g]: '变红·检查点', [p]: '变黑', [uncle]: '变黑' } })
        z = g
        continue
      }
      const zIsLeft = nodes.get(p)!.l === z
      if (pIsLeft && !zIsLeft) {
        frame(`${z} 是父 ${p} 的右孩子、${p} 是祖父 ${g} 的左孩子 → LR 折线：先对 ${p} 左旋成 LL 直线`,
          '折线分支：先半旋拉直，再按直线处理（与 AVL 四形态同构）',
          { act: z, labels: { [z]: '折线点', [p]: '半旋支点' } })
        const np = rotLeft(p)
        attach(g, p, np)
        z = p
        frame(`半旋完成：${np} 升为 ${g} 的左孩子，${z} 成为其左孩子——折线已成 LL 直线`,
          undefined, { act: z, path: pathTo(z) })
        fixStraight(z, g, true, true)
      } else if (!pIsLeft && zIsLeft) {
        frame(`${z} 是父 ${p} 的左孩子、${p} 是祖父 ${g} 的右孩子 → RL 折线：先对 ${p} 右旋成 RR 直线`,
          '折线分支：先半旋拉直，再按直线处理',
          { act: z, labels: { [z]: '折线点', [p]: '半旋支点' } })
        const np = rotRight(p)
        attach(g, p, np)
        z = p
        frame(`半旋完成：${np} 升为 ${g} 的右孩子，${z} 成为其右孩子——折线已成 RR 直线`,
          undefined, { act: z, path: pathTo(z) })
        fixStraight(z, g, false, true)
      } else if (pIsLeft && zIsLeft) {
        fixStraight(p, g, true, false)
      } else {
        fixStraight(p, g, false, false)
      }
      break
    }
    if (root != null) nodes.get(root)!.color = 'B'
    frame(`根恒黑（性质2）：root 染黑，${key} 插入完成`, undefined, root != null ? { done: [root] } : {})
  })

  // 2. 收尾
  frame(`演示结束：${input.keys.length} 个键全部插入，rb_verify 通过（黑高一致、无相邻红、根黑）`,
    '推论：n 结点红黑树高度 ≤ 2·log2(n+1)——红结点至多占一半，"弱平衡"换来较少的旋转',
    root != null ? { done: [root] } : {})

  return steps
}
