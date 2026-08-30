// docs/.vitepress/theme/visualizer/steps/avlRotate.ts
import type { Step } from '../types'
import { tstep, bin } from './treeKit'

// AVL 插入触发旋转（对应 08_查找/05_平衡二叉树/avl.c insert_rec + rebalance）：
//   递归插入，回溯时逐结点 upd_height + rebalance：
//     bf = h(左) - h(右)；bf > 1 且 h(l.l) >= h(l.r) → LL 直接右旋；
//     bf > 1 且 h(l.l) <  h(l.r) → LR 先左旋变 LL 再右旋；bf < -1 对称（RR / RL）。
//   失衡点 = 回溯路上第一个 |bf| > 1 的结点（最小失衡子树根），每次插入至多一次旋转。
// 输入插入串（3~6 个键，validate 要求确实触发旋转，否则提示可触发的样例序列）。
export interface AvlInput { keys: number[] }

/** validate 用：无帧模拟 avl.c 插入，返回旋转总次数（每次插入至多 1 次，双旋算 1 次调整） */
export function avlRotationCount(keys: number[]): number {
  const nd = new Map<number, { l: number | null; r: number | null; h: number }>()
  let root: number | null = null
  const h = (id: number | null) => (id == null ? 0 : nd.get(id)!.h)
  const upd = (id: number) => { const p = nd.get(id)!; p.h = 1 + Math.max(h(p.l), h(p.r)) }
  const rotR = (x: number): number => { const xn = nd.get(x)!; const y = xn.l as number; const yn = nd.get(y)!; xn.l = yn.r; yn.r = x; upd(x); upd(y); return y }
  const rotL = (x: number): number => { const xn = nd.get(x)!; const y = xn.r as number; const yn = nd.get(y)!; xn.r = yn.l; yn.l = x; upd(x); upd(y); return y }
  const reb = (x: number): number => {
    const p = nd.get(x)!
    const bf = h(p.l) - h(p.r)
    if (bf > 1) {
      const y = p.l as number
      if (h(nd.get(y)!.l) >= h(nd.get(y)!.r)) return rotR(x)
      p.l = rotL(y)
      return rotR(x)
    }
    if (bf < -1) {
      const y = p.r as number
      if (h(nd.get(y)!.r) >= h(nd.get(y)!.l)) return rotL(x)
      p.r = rotR(y)
      return rotL(x)
    }
    return x
  }
  let rotations = 0
  for (const key of keys) {
    // 递归插入 + 回溯（栈实现）
    const stack: number[] = []
    let cur = root
    while (cur != null) {
      if (key === cur) return rotations // 重复：validate 层先拒，防御
      stack.push(cur)
      cur = key < cur ? nd.get(cur)!.l : nd.get(cur)!.r
    }
    nd.set(key, { l: null, r: null, h: 1 })
    if (stack.length === 0) { root = key; continue }
    const parent = stack[stack.length - 1]
    const pn = nd.get(parent)!
    if (key < parent) pn.l = key
    else pn.r = key
    while (stack.length) {
      const p = stack.pop() as number
      upd(p)
      const nb = reb(p)
      if (nb !== p) rotations++
      if (stack.length) {
        const gp = nd.get(stack[stack.length - 1] as number)!
        if (gp.l === p) gp.l = nb
        else gp.r = nb
      } else {
        root = nb
      }
    }
  }
  return rotations
}

interface AN { key: number; l: number | null; r: number | null; h: number }

export function avlRotateSteps(input: AvlInput): Step[] {
  const steps: Step[] = []
  const nodes = new Map<number, AN>()
  let root: number | null = null
  const h = (id: number | null) => (id == null ? 0 : nodes.get(id)!.h)
  const upd = (id: number) => {
    const nd = nodes.get(id)!
    nd.h = 1 + Math.max(h(nd.l), h(nd.r))
  }
  const kidsOf = (id: number): number[] => {
    const nd = nodes.get(id)!
    return [nd.l, nd.r].filter((x): x is number => x != null)
  }

  /** 全树帧；labels 覆盖 nodeLabel（默认 h 标注），act 点名，done 绿色，path 高亮比较路径 */
  const frame = (narration: string, note?: string, opt: {
    labels?: Record<number, string>; act?: number; done?: number[]; path?: number[]
  } = {}) => {
    const out = [] as ReturnType<typeof bin>[]
    const draw = (id: number): void => {
      const nd = nodes.get(id)!
      const kids = kidsOf(id)
      const pi = (opt.path ?? []).indexOf(id)
      out.push(bin(id, nd.key, nd.l, nd.r, {
        nodeLabel: opt.labels?.[id] ?? `h=${nd.h}`,
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

  const bfLabel = (id: number) => {
    const nd = nodes.get(id)!
    return `h=${nd.h} BF=${h(nd.l) - h(nd.r)}`
  }

  // 旋转原子操作（avl.c rot_right / rot_left 逐行同构）
  const rotRight = (x: number): number => {
    const xn = nodes.get(x)!
    const y = xn.l as number
    const yn = nodes.get(y)!
    xn.l = yn.r
    yn.r = x
    upd(x)
    upd(y)
    return y
  }
  const rotLeft = (x: number): number => {
    const xn = nodes.get(x)!
    const y = xn.r as number
    const yn = nodes.get(y)!
    xn.r = yn.l
    yn.l = x
    upd(x)
    upd(y)
    return y
  }

  /** 对失衡结点 p 做四形态调整，返回新子树根；期间产出旋转前后帧 */
  const rebalance = (p: number, parentOf: number | null): number => {
    const bf = h(nodes.get(p)!.l) - h(nodes.get(p)!.r)
    if (bf > 1) {
      const y = nodes.get(p)!.l as number
      const yn = nodes.get(y)!
      if (h(yn.l) >= h(yn.r)) {
        frame(`失衡点 ${p}：BF = h(左)−h(右) = ${bf} > 1，且 h(${y} 的左)=${h(yn.l)} ≥ h(${y} 的右)=${h(yn.r)} → LL 形态，直接右单旋`,
          'LL 形态：右单旋。x 的左孩子 y 升为根，x 挂到 y 右侧（rot_right）',
          { labels: { [p]: '旧根·失衡', [y]: '新根·升位' }, act: y, path: parentOf != null ? pathTo(parentOf) : [] })
        const yOldR = yn.r
        const nr = rotRight(p)
        attach(parentOf, p, nr)
        frame(`rot_right 完成：${y} 升为子树根，${p} 挂到 ${y} 右侧（${y} 原右孩子${yOldR != null ? ` ${yOldR}` : '为空'}由 ${p} 接管当左孩子）——子树高度回落，恢复平衡`,
          undefined, { labels: { [y]: '新根', [p]: '降位' }, act: y })
        return nr
      }
      const yl = yn.r as number
      frame(`失衡点 ${p}：BF = ${bf} > 1，但 h(${y} 的左)=${h(yn.l)} < h(${y} 的右)=${h(yl)} → LR 折线，先对 ${y} 左旋变 LL，再对 ${p} 右旋`,
        'LR 形态：两次旋转。左旋把折线拉直成 LL，再右单旋',
        { labels: { [p]: '旧根·失衡', [y]: '折线点' }, act: y, path: parentOf != null ? pathTo(parentOf) : [] })
      const ny = rotLeft(y)
      nodes.get(p)!.l = ny
      frame(`第一次 rot_left(${y})：${ny} 升为 ${p} 的左孩子，${y} 挂到 ${ny} 左侧——折线已成 LL 直线`,
        undefined, { labels: { [p]: '待右旋', [ny]: '直线点' }, act: ny })
      const nr = rotRight(p)
      attach(parentOf, p, nr)
      frame(`第二次 rot_right(${p})：${ny} 升为子树根（LR 的新根就是折线点），${p} 挂其右侧——恢复平衡`,
        undefined, { labels: { [ny]: '新根', [p]: '降位' }, act: ny })
      return nr
    }
    if (bf < -1) {
      const y = nodes.get(p)!.r as number
      const yn = nodes.get(y)!
      if (h(yn.r) >= h(yn.l)) {
        frame(`失衡点 ${p}：BF = ${bf} < -1，且 h(${y} 的右)=${h(yn.r)} ≥ h(${y} 的左)=${h(yn.l)} → RR 形态，直接左单旋`,
          'RR 形态：左单旋。x 的右孩子 y 升为根，x 挂到 y 左侧（rot_left）',
          { labels: { [p]: '旧根·失衡', [y]: '新根·升位' }, act: y, path: parentOf != null ? pathTo(parentOf) : [] })
        const yOldL = yn.l
        const nr = rotLeft(p)
        attach(parentOf, p, nr)
        frame(`rot_left 完成：${y} 升为子树根，${p} 挂到 ${y} 左侧（${y} 原左孩子${yOldL != null ? ` ${yOldL}` : '为空'}由 ${p} 接管当右孩子）——子树高度回落，恢复平衡`,
          undefined, { labels: { [y]: '新根', [p]: '降位' }, act: y })
        return nr
      }
      const yr = yn.l as number
      frame(`失衡点 ${p}：BF = ${bf} < -1，但 h(${y} 的右)=${h(yn.r)} < h(${y} 的左)=${h(yr)} → RL 折线，先对 ${y} 右旋变 RR，再对 ${p} 左旋`,
        'RL 形态：两次旋转。右旋把折线拉直成 RR，再左单旋',
        { labels: { [p]: '旧根·失衡', [y]: '折线点' }, act: y, path: parentOf != null ? pathTo(parentOf) : [] })
      const ny = rotRight(y)
      nodes.get(p)!.r = ny
      frame(`第一次 rot_right(${y})：${ny} 升为 ${p} 的右孩子，${y} 挂到 ${ny} 右侧——折线已成 RR 直线`,
        undefined, { labels: { [p]: '待左旋', [ny]: '直线点' }, act: ny })
      const nr = rotLeft(p)
      attach(parentOf, p, nr)
      frame(`第二次 rot_left(${p})：${ny} 升为子树根（RL 的新根就是折线点），${p} 挂其左侧——恢复平衡`,
        undefined, { labels: { [ny]: '新根', [p]: '降位' }, act: ny })
      return nr
    }
    return p
  }

  const attach = (parentOf: number | null, oldChild: number, newChild: number) => {
    if (parentOf == null) { root = newChild; return }
    const pn = nodes.get(parentOf)!
    if (pn.l === oldChild) pn.l = newChild
    else pn.r = newChild
  }
  const pathTo = (target: number): number[] => {
    const out: number[] = []
    let cur: number | null = root
    while (cur != null) { out.push(cur); if (cur === target) break; cur = target > cur ? nodes.get(cur)!.r : nodes.get(cur)!.l }
    return out
  }

  // 0. 开场
  frame(`AVL 插入演示：${input.keys.join(',')}。插入后回溯逐结点查 BF（= h(左)−h(右)），失衡即旋转`,
    'AVL 性质：任一结点左右子树高度差 ∈ [-1,1]；四旋转 LL/RR 单旋、LR/RL 双旋，插入至多一次旋转')

  // 1. 逐键插入
  let rotated = false
  input.keys.forEach((key) => {
    if (root == null) {
      nodes.set(key, { key, l: null, r: null, h: 1 })
      root = key
      frame(`空树：${key} 成为根（h=1）`, undefined, { act: key })
      return
    }
    // 下落
    const path: number[] = []
    let cur: number | null = root
    while (cur != null) {
      path.push(cur)
      frame(`${key} vs ${cur}：${key} ${key < cur ? '<' : key > cur ? '>' : '='} ${cur} → 走${key < cur ? '左' : key > cur ? '右' : '无（重复）'}`,
        undefined, { act: cur, path })
      if (key === cur) return // 重复：validate 已拒，防御
      cur = key < cur ? nodes.get(cur)!.l : nodes.get(cur)!.r
    }
    const parent = path[path.length - 1]
    nodes.set(key, { key, l: null, r: null, h: 1 })
    const pn = nodes.get(parent)!
    if (key < pn.key) pn.l = key
    else pn.r = key
    frame(`挂新叶子：${parent} 的${key < pn.key ? '左' : '右'}孩子 = ${key}（h=1），开始回溯查 BF`,
      undefined, { act: key, path: [...path, key] })
    // 回溯：逐结点 upd_height + rebalance（avl.c insert_rec 回溯段）
    for (let i = path.length - 1; i >= 0; i--) {
      const p = path[i]
      upd(p)
      const nd = nodes.get(p)!
      const bf = h(nd.l) - h(nd.r)
      const parentOf = i > 0 ? path[i - 1] : null
      if (Math.abs(bf) <= 1) {
        frame(`回溯 ${p}：${bfLabel(p)}，|BF| ≤ 1 不失衡${i > 0 ? '，继续向上' : '，到根了'}`,
          undefined, { labels: { [p]: bfLabel(p) }, act: p, path: pathTo(p) })
      } else {
        frame(`回溯 ${p}：${bfLabel(p)}，|BF| > 1 —— 最小失衡子树根找到了`,
          undefined, { labels: { [p]: bfLabel(p) + ' 失衡' }, act: p, path: pathTo(p) })
        rebalance(p, parentOf)
        rotated = true
      }
    }
  })

  // 2. 收尾
  const labels: Record<number, string> = {}
  for (const [id] of nodes) labels[id] = bfLabel(id)
  const sorted = inorderKeys()
  frame(rotated
    ? `演示结束：旋转后全部 |BF| ≤ 1（avl_verify violations=0），中序仍升序 ${sorted}——旋转不改中序序`
    : `演示结束：全程无失衡（validate 本要求至少一次旋转，此帧仅防御）`,
    '旋转前后两帧点明类型：LL/RR 单旋换一根，LR/RL 双旋新根是折线点', { labels })

  return steps

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
