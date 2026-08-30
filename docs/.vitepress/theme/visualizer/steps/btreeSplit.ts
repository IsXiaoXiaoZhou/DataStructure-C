// docs/.vitepress/theme/visualizer/steps/btreeSplit.ts
import type { Step } from '../types'
import { tstep, type TNode } from './treeKit'

// B 树插入与分裂（对应 08_查找/07_B树/btree.c，CLRS 风格"先分裂再下降"）：
//   最小度 t = BTREE_T = 3：每结点键数 ∈ [t-1, 2t-1] = [2,5]，孩子数 = 键数+1。
//   split_child(x, i)：满孩子 y（5 键）裂成两半——左留 t-1=2 键、中键 keys[t-1] 上移给 x、
//   右得 ⌊(2t-1)/2⌋=2 键；内部结点连带分孩子（左 t=3 右 t=3）。
//   根满先建新根上浮，树增一层；叶子内有序插入。
// 输入 6~8 个键（t=3 下单结点至多 5 键，第 6 键必触发根分裂）。
export interface BtreeInput { keys: number[] }

const T = 3
const MAX_KEYS = 2 * T - 1 // 5

interface BTN { id: number; keys: number[]; ch: (number | null)[]; leaf: boolean }

export function btreeSplitSteps(input: BtreeInput): Step[] {
  const steps: Step[] = []
  const nodes = new Map<number, BTN>()
  let root: number | null = null
  let nextId = 1

  const newNode = (leaf: boolean): number => {
    const id = nextId++
    nodes.set(id, { id, keys: [], ch: leaf ? [] : [null], leaf })
    return id
  }

  /** 全树帧；keyAct = {结点: 高亮键位集}，edgeAct = {结点: 高亮孩子下标}，labels 覆盖 */
  const frame = (narration: string, note?: string, opt: {
    keyAct?: Record<number, number[]>; edgeAct?: Record<number, number[]>; labels?: Record<number, string>
  } = {}) => {
    const out: TNode[] = [...nodes.values()].map(nd => ({
      id: nd.id,
      keys: [...nd.keys],
      children: nd.ch.filter((c): c is number => c != null),
      nodeLabel: opt.labels?.[nd.id] ?? (nd.leaf ? '叶' : '内'),
      keyActive: opt.keyAct?.[nd.id],
      edgeActive: opt.edgeAct?.[nd.id]
    }))
    steps.push(tstep(out, root, narration, note))
  }

  const locate = (x: BTN, key: number) => {
    let i = 0
    while (i < x.keys.length && x.keys[i] < key) i++
    return i
  }

  /** 分裂 x 的第 i 个满孩子（btree.c split_child 逐行同构） */
  const splitChild = (x: BTN, i: number) => {
    const y = nodes.get(x.ch[i] as number)!
    const z = nodes.get(newNode(y.leaf))!
    const half = Math.floor(MAX_KEYS / 2) // (2t-1)/2 = t-1 = 2 键给右
    z.keys = y.keys.slice(T, T + half)    // y 后半键
    if (!y.leaf) {
      z.ch = y.ch.slice(T, T + half + 1)  // 连带走孩子（半+1 个）
    }
    const mid = y.keys[T - 1]             // 中键上移
    y.keys = y.keys.slice(0, T - 1)
    if (!y.leaf) y.ch = y.ch.slice(0, T)
    x.keys.splice(i, 0, mid)
    x.ch.splice(i + 1, 0, z.id)
    return { y, z, mid }
  }

  // 0. 开场
  frame(`B 树插入演示（最小度 t=${T}）：单结点键数 ∈ [t−1, 2t−1] = [${T - 1}, ${MAX_KEYS}]，孩子数 = 键数 + 1；策略是"先分裂再下降"（CLRS 自顶向下）`,
    '性质：所有叶子同层（查找路径等长）；结点内键升序，孩子子树间有序（多路 BST）')

  const inorderKeys = (): string => {
    const out: number[] = []
    const rec = (id: number | null) => {
      if (id == null) return
      const nd = nodes.get(id)!
      if (nd.leaf) { out.push(...nd.keys); return }
      for (let j = 0; j < nd.keys.length; j++) {
        rec(nd.ch[j] ?? null)
        out.push(nd.keys[j])
      }
      rec(nd.ch[nd.keys.length] ?? null)
    }
    rec(root)
    return out.join(', ')
  }

  /** 向未满结点插入（btree.c insert_nonfull 同构，带帧） */
  const insertNonfull = (xId: number, key: number): 'ok' | 'dup' => {
    const x = nodes.get(xId)!
    let i = locate(x, key)
    if (i < x.keys.length && x.keys[i] === key) {
      frame(`key = ${x.keys[i]}：关键字已存在（DS_ERROR）——B 树同结点内也要查重`,
        undefined, { keyAct: { [x.id]: [i] } })
      return 'dup'
    }
    if (x.leaf) {
      x.keys.splice(i, 0, key)
      frame(`叶子有序插入 ${key}：腾位写入第 ${i + 1} 格 → [${x.keys.join(', ')}]（结点现 ${x.keys.length} 键）`,
        undefined, { keyAct: { [x.id]: [i] }, labels: { [x.id]: `叶·${x.keys.length} 键` } })
      return 'ok'
    }
    // 内部结点：孩子满则先分裂再下降
    let childId = x.ch[i] as number
    if (nodes.get(childId)!.keys.length === MAX_KEYS) {
      frame(`下降途中：${x.id} 号的第 ${i + 1} 个孩子（[${nodes.get(childId)!.keys.join(', ')}]）已满 ${MAX_KEYS} 键 = 2t−1 → 先分裂再下降`,
        '先分裂再下降：一路下来保证落到的是未满结点，回溯无需再调整（与递归分裂法相对）',
        { keyAct: { [childId]: nodes.get(childId)!.keys.map((_, j) => j) }, labels: { [childId]: '满·将裂' } })
      const { y, z, mid } = splitChild(x, i)
      frame(`split_child：中键 ${mid} 上移给父 ${x.id}，左留 t−1=${T - 1} 键 [${y.keys.join(', ')}]，右得 ⌊(2t−1)/2⌋=${Math.floor(MAX_KEYS / 2)} 键 [${z.keys.join(', ')}]${y.leaf ? '' : `；内部结点连带分孩子（左 ${y.ch.length} 右 ${z.ch.length} 个）`}——新结点 ${z.id} 号`,
        `分裂三份：左 ${T - 1} 键 / 中键上浮 / 右 ${Math.floor(MAX_KEYS / 2)} 键；结点键数始终落在 [${T - 1}, ${MAX_KEYS}]`,
        { keyAct: { [x.id]: [x.keys.indexOf(mid)], [z.id]: [] }, labels: { [y.id]: `左·${y.keys.length} 键`, [z.id]: `右·新结点`, [x.id]: '父·收中键' } })
      i = locate(x, key) // 分裂后重新定位（可能中键恰好等于 key，validate 已拒重复）
      if (i < x.keys.length && x.keys[i] === key) return 'dup'
      childId = x.ch[i] as number
    }
    frame(`在 ${x.id} 号结点定位：key=${key} 与 [${x.keys.join(', ')}] 线性比较 → 走第 ${i + 1} 棵子树（${childId} 号）`,
      undefined, { edgeAct: { [x.id]: [i] }, labels: { [x.id]: '内' } })
    return insertNonfull(childId, key)
  }

  input.keys.forEach((key, ki) => {
    frame(`插入 ${key}（第 ${ki + 1}/${input.keys.length} 个）`, undefined)
    if (root == null) {
      const r = newNode(true)
      nodes.get(r)!.keys = [key]
      root = r
      frame(`首键建根：${key} 写入新叶根 ${r} 号（btree_insert 的 *root == NULL 分支）`,
        undefined, { keyAct: { [r]: [0] } })
      return
    }
    const rn = nodes.get(root)!
    if (rn.keys.length === MAX_KEYS) {
      // 根满：新根上浮，树增一层
      frame(`根满（[${rn.keys.join(', ')}]，${MAX_KEYS} = 2t−1 键）→ 建新根上浮，树增一层`,
        '根分裂是 B 树长高的唯一方式——所有叶子同层由此保证',
        { keyAct: { [rn.id]: rn.keys.map((_, j) => j) }, labels: { [rn.id]: '满·将裂' } })
      const s = newNode(false)
      nodes.get(s)!.ch = [root]
      root = s
      const { y, z, mid } = splitChild(nodes.get(s)!, 0)
      frame(`根分裂：新根 ${s} 号收中键 ${mid}；左 [${y.keys.join(', ')}] / 右 [${z.keys.join(', ')}] 两个孩子——树高 +1`,
        undefined, { keyAct: { [s]: [0] }, labels: { [s]: '新根', [y.id]: '左', [z.id]: '右' } })
    }
    insertNonfull(root, key)
  })

  frame(`演示结束：${input.keys.length} 个键插入完成，中序遍历 = ${inorderKeys()}（升序）`,
    `最终多叉树形：根 ${nodes.get(root!)?.keys.length ?? 0} 键 ${nodes.get(root!)?.ch.filter(c => c != null).length ?? 0} 棵子树；每个内结点 keys[i] 把孩子子树按"小于 keys[i] / 大于 keys[i]"切开`)

  return steps
}
