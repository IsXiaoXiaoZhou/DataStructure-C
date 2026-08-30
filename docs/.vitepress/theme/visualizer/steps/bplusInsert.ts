// docs/.vitepress/theme/visualizer/steps/bplusInsert.ts
import type { Step } from '../types'
import { tstep, type TNode } from './treeKit'

// B+ 树插入（对应 08_查找/08_B+树/bplus.c，按其真实变体）：
//   n 键配 n+1 子；内部结点只存路由键 = 右子树（ch[i+1]）最小键，ch[0] 子树全 < keys[0]；
//   所有键存于叶层，叶经 ch[0]（即 next）串成升序链；上界 2t 键（BPT_MAX = 2*BPT_T = 6），
//   到达即分裂：叶裂成 t + t 键、右叶首键"复制"上浮为路由键（键必须留在叶层）；
//   内部裂成 t 键 + (t-1) 键、中键 keys[t] 上移（路由键移动）。
//   普通插入不更新路由键：更小的键总落在 ch[0] 侧（比一切路由键小）。
// 输入 5~14 个键（≥6 必触发叶分裂，默认串另含第二次分裂与内部路由）。
export interface BplusInput { keys: number[] }

const T = 3
const MAX = 2 * T // 6：结点最多键数，到达即分裂

interface BPN { id: number; keys: number[]; ch: (number | null)[]; leaf: boolean }

export function bplusInsertSteps(input: BplusInput): Step[] {
  const steps: Step[] = []
  const nodes = new Map<number, BPN>()
  let root: number | null = null
  let nextId = 1

  const newNode = (leaf: boolean): number => {
    const id = nextId++
    nodes.set(id, { id, keys: [], ch: leaf ? [null] : [], leaf }) // 叶 ch[0] = next 链指针
    return id
  }

  /** 叶链（最左叶起沿 next） */
  const leafChain = (): number[] => {
    let cur: number | null = root
    while (cur != null && !nodes.get(cur)!.leaf) cur = nodes.get(cur)!.ch[0]
    const out: number[] = []
    let guard = nodes.size + 1
    while (cur != null && guard-- > 0) {
      out.push(cur)
      cur = nodes.get(cur)!.ch[0]
    }
    return out
  }

  const frame = (narration: string, note?: string, opt: {
    keyAct?: Record<number, number[]>; edgeAct?: Record<number, number[]>; labels?: Record<number, string>
  } = {}) => {
    const out: TNode[] = [...nodes.values()].map(nd => ({
      id: nd.id,
      keys: [...nd.keys],
      // 叶的 ch[0] 是 next 链指针不是孩子：叶画成无孩子的键集结点（叶链在 note 说明）
      children: nd.leaf ? [] : nd.ch.filter((c): c is number => c != null && nodes.has(c)),
      nodeLabel: opt.labels?.[nd.id] ?? (nd.leaf ? '叶' : '内·路由'),
      keyActive: opt.keyAct?.[nd.id],
      edgeActive: opt.edgeAct?.[nd.id]
    }))
    steps.push(tstep(out, root, narration, note))
  }
  /** 帧变体：叶链写进 note */
  type FrameOpt = { keyAct?: Record<number, number[]>; edgeAct?: Record<number, number[]>; labels?: Record<number, string> }
  const frameChain = (narration: string, note?: string, opt: FrameOpt = {}) => {
    const chain = leafChain().map(id => `[${nodes.get(id)!.keys.join(',')}]`).join(' → ')
    frame(narration, note ? `${note}；叶链：${chain}` : `叶链：${chain}`, opt)
  }

  const locate = (x: BPN, key: number) => {
    let i = 0
    while (i < x.keys.length && x.keys[i] < key) i++
    return i
  }

  /** 递归插入：返回 [上浮路由键(0=无), 新右兄弟] */
  const insertRec = (xId: number, key: number, depth: number): { route: number; right: number | null } => {
    const x = nodes.get(xId)!
    if (x.leaf) {
      const dup = x.keys.indexOf(key)
      if (dup >= 0) {
        frame(`key = ${key} 已在叶 [${x.keys.join(', ')}]：关键字已存在（DS_ERROR）`,
          undefined, { keyAct: { [x.id]: [dup] } })
        return { route: 0, right: null }
      }
      let i = x.keys.length
      x.keys.push(key)
      while (i > 0 && x.keys[i - 1] > key) { x.keys[i] = x.keys[i - 1]; i-- } // 有序腾位（leaf_insert）
      x.keys[i] = key
      frame(`叶 ${x.id} 有序插入 ${key} → [${x.keys.join(', ')}]（现 ${x.keys.length} 键${x.keys.length >= MAX ? '，已达上界 2t' : ''}）`,
        undefined, { keyAct: { [x.id]: [i] }, labels: { [x.id]: `叶·${x.keys.length} 键` } })
      if (x.keys.length < MAX) return { route: 0, right: null }
      // 叶分裂：左 T 键、右 MAX−T 键，右叶首键复制上浮（挂接帧由调用方出，保证画面连通）
      frame(`叶满（${x.keys.length} = 2t 键 [${x.keys.join(', ')}]）→ 分裂：左 t=${T} 键 / 右 ${MAX - T} 键，右叶首键"复制"上浮为路由键`,
        '与 B 树不同：路由键是复制不是移动——B+ 树所有键必须留在叶层，内部只存路由键',
        { keyAct: { [x.id]: x.keys.map((_, j) => j) }, labels: { [x.id]: '满·将裂' } })
      const right = nodes.get(newNode(true))!
      right.keys = x.keys.slice(T)
      const route = right.keys[0]
      x.keys = x.keys.slice(0, T)
      right.ch[0] = x.ch[0]        // 叶链接续：新右叶接住原 next
      x.ch[0] = right.id           // 本叶 next 指向新右叶
      return { route, right: right.id }
    }
    // 内部结点：定位（等值路由走右）
    let ci = locate(x, key)
    const descNote = ci < x.keys.length
      ? (x.keys[ci] === key
        ? `${key} 等于路由键 ${x.keys[ci]} → ci 后移走右侧`
        : `${key} < 路由键 ${x.keys[ci]} → 走第 ${ci + 1} 棵子树（其键全 < ${x.keys[ci]}）`)
      : `${key} 大于所有路由键 → 走最右子树`
    frame(`内部 ${x.id} 定位：${descNote}（路由键 = 右子树最小键）`,
      depth === 1 ? '查找必到达叶层——内部键只是路标，不存数据' : undefined,
      { edgeAct: { [x.id]: [Math.min(ci, x.keys.length)] } })
    if (ci < x.keys.length && x.keys[ci] === key) ci++
    const res = insertRec(x.ch[ci] as number, key, depth + 1)
    if (res.route === 0 || res.right == null) return { route: 0, right: null }
    // 上浮路由键插入本结点（分裂的右兄弟随本帧一并挂接，画面保持连通）
    x.keys.splice(ci, 0, res.route)
    x.ch.splice(ci + 1, 0, res.right)
    frame(`路由键 ${res.route} 上浮插入内部 ${x.id} → [${x.keys.join(', ')}]（孩子 ${x.ch.length} 个 = 键数 + 1），分裂出的右兄弟 ${res.right} 随之挂接`,
      undefined, { keyAct: { [x.id]: [ci] }, labels: { [x.id]: `内·${x.keys.length} 键` } })
    if (x.keys.length < MAX) return { route: 0, right: null }
    // 内部分裂：中键 keys[t] 上移（移动，非复制），左 t 键 / 右 t−1 键（挂接帧由调用方出）
    frame(`内部满（${x.keys.length} = 2t 键）→ 分裂：中键 keys[t] 上移，左 t=${T} 键 / 右 t−1=${T - 1} 键（连带分孩子）`,
      undefined, { keyAct: { [x.id]: x.keys.map((_, j) => j) }, labels: { [x.id]: '满·将裂' } })
    const right = nodes.get(newNode(false))!
    const mid = x.keys[T]
    right.keys = x.keys.slice(T + 1)
    right.ch = x.ch.slice(T + 1)
    x.keys = x.keys.slice(0, T)
    x.ch = x.ch.slice(0, T + 1)
    return { route: mid, right: right.id }
  }

  // 0. 开场
  frame(`B+ 树插入演示（最小度 t=${T}，结点上界 2t=${MAX} 键）：n 键配 n+1 子，内部只存路由键 = 右子树最小键，所有键在叶层`,
    '与 B 树区别：键全在叶、内部存路由键（= 右子树最小键）、叶串升序链——范围查询沿叶链扫描 O(k + log_t n)')

  input.keys.forEach((key, ki) => {
    frameChain(`插入 ${key}（第 ${ki + 1}/${input.keys.length} 个）`)
    if (root == null) {
      const r = newNode(true)
      nodes.get(r)!.keys = [key]
      root = r
      frame(`首键建叶根：${key} 写入 ${r} 号叶`, undefined, { keyAct: { [r]: [0] } })
      return
    }
    const res = insertRec(root, key, 1)
    if (res.route !== 0 && res.right != null) {
      // 根分裂：新根收路由键（bpt_insert 尾段）
      const nr = newNode(false)
      nodes.get(nr)!.keys = [res.route]
      nodes.get(nr)!.ch = [root, res.right]
      root = nr
      frame(`根分裂收尾：新根 ${nr} 号 = [${res.route}]，老根与新右叶/右半成为它的两棵子树——树高 +1`,
        undefined, { keyAct: { [nr]: [0] }, labels: { [nr]: '新根' } })
    }
  })

  const chain = leafChain().map(id => `[${nodes.get(id)!.keys.join(',')}]`).join(' → ')
  frameChain(`演示结束：${input.keys.length} 个键插入完成——普通插入不更新路由键：更小的键总落在 ch[0] 侧（比一切路由键小），路由键只在分裂时产生`,
    `范围查询到最左叶后沿链扫描`)

  return steps
}
