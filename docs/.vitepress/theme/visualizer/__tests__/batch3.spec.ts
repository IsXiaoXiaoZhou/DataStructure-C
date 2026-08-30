// docs/.vitepress/theme/visualizer/__tests__/batch3.spec.ts
// 批3：Tree 渲染器与树类 12 个动画（存储视图/顺序编号/遍历/线索/森林/哈夫曼/并查集/BST/AVL/红黑/B 树/B+ 树）
import { describe, it, expect } from 'vitest'
import type { Step } from '../types'
import { registry } from '../registry'

import { treeStorageViewsSteps } from '../steps/treeStorageViews'
import { seqBitreeIndexSteps } from '../steps/seqBitreeIndex'
import { bitreeTraverseSteps } from '../steps/bitreeTraverse'
import { threadBuildSteps } from '../steps/threadBuild'
import { forestConvertSteps } from '../steps/forestConvert'
import { huffmanBuildSteps } from '../steps/huffmanBuild'
import { unionFindOpsSteps } from '../steps/unionFindOps'
import { bstInsertSearchSteps } from '../steps/bstInsertSearch'
import { avlRotateSteps } from '../steps/avlRotate'
import { rbInsertSteps } from '../steps/rbInsert'
import { btreeSplitSteps } from '../steps/btreeSplit'
import { bplusInsertSteps } from '../steps/bplusInsert'

const clone = <T,>(x: T): T => structuredClone(x)
const last = (steps: Step[]): Step => steps[steps.length - 1]
const allNarration = (steps: Step[]) => steps.map(s => s.narration).join('\n')

/** 纯函数：同输入两次运行结果一致，且不改输入 */
function expectPure<T>(fn: (x: T) => Step[], input: T) {
  const before = clone(input)
  const a = fn(input)
  const b = fn(input)
  expect(input).toEqual(before)   // 不修改输入
  expect(a).toEqual(b)            // 输出确定性
}

/** Tree state 契约：id 唯一、children id 必存在、无环且全连通（rootId 可达全部结点）、
 *  键位/边高亮不越界、color 合法；rootId null 仅允许空树帧（nodes 为空） */
function expectTreeContract(steps: Step[]) {
  expect(steps.length, '帧数不足').toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length, 'narration 非空').toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.nodes)).toBe(true)
    expect(o.rootId === null || typeof o.rootId === 'number').toBe(true)
    const ids = new Set<number>(o.nodes.map((n: any) => n.id))
    expect(ids.size).toBe(o.nodes.length)
    for (const n of o.nodes) {
      expect(typeof n.id).toBe('number')
      expect(Array.isArray(n.keys)).toBe(true)
      expect(Array.isArray(n.children)).toBe(true)
      for (const c of n.children) expect(ids.has(c), `结点 ${n.id} 的孩子 ${c} 悬空`).toBe(true)
      expect(['R', 'B', undefined]).toContain(n.color)
      for (const k of [...(n.keyActive ?? []), ...(n.keyHighlight ?? [])]) {
        expect(k).toBeGreaterThanOrEqual(0)
        expect(k, `结点 ${n.id} 键位高亮越界`).toBeLessThan(n.keys.length)
      }
      for (const e of n.edgeActive ?? []) {
        expect(e).toBeGreaterThanOrEqual(0)
        expect(e, `结点 ${n.id} 边高亮越界`).toBeLessThan(n.children.length)
      }
    }
    if (o.rootId === null) {
      expect(o.nodes.length, '空树帧不得携带结点').toBe(0)
      continue
    }
    expect(ids.has(o.rootId), 'rootId 悬空').toBe(true)
    // 连通 + 无环（每个结点恰被一个双亲指到）
    const seen = new Set<number>()
    const stack = [o.rootId as number]
    while (stack.length) {
      const cur = stack.pop() as number
      expect(seen.has(cur), `结点 ${cur} 被重复指为孩子（环/共享孩子）`).toBe(false)
      seen.add(cur)
      const node = o.nodes.find((n: any) => n.id === cur)
      for (const c of node?.children ?? []) stack.push(c)
    }
    expect(seen.size, '树不连通：存在 rootId 不可达的结点').toBe(o.nodes.length)
  }
}

/** 从访问帧 narration 提取逐结点访问序（匹配 "访问 X（第 k/total 个"） */
function visitOrder(steps: Step[], modeName: string): string[] {
  return steps
    .map(s => s.narration.match(new RegExp(`${modeName}访问 (\\S+?)（第 \\d+/\\d+ 个`)))
    .filter(Boolean)
    .map(m => m![1])
}

// ---------- 1. tree-storage-views ----------
describe('treeStorageViews 生成器', () => {
  const input = { values: [1, 2, 3, 4, 5] }
  it('终帧：孩子兄弟二叉形态——1 左 2；2 左 4 右 3；4 右 5；narration 给代价对照', () => {
    const steps = treeStorageViewsSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(1).children).toEqual([2])
    expect(byId.get(2).children).toEqual([4, 3])
    expect(byId.get(4).children).toEqual([5])
    expect(byId.get(3).children).toEqual([])
    expect(byId.get(5).children).toEqual([])
    const all = allNarration(steps)
    expect(all).toContain('O(1)')
    expect(all).toContain('全表扫描')
    expect(all).toContain('沿链')
    expectTreeContract(steps)
  })
  it('双亲表示帧 nodeLabel 给 parent 下标；孩子链帧给链序；空链域帧给出 n+1 = 6', () => {
    const steps = treeStorageViewsSteps(input)
    const labelSets = steps.map(s => (s.state as any).nodes.map((n: any) => n.nodeLabel ?? '').join('|'))
    expect(labelSets.some(l => l.includes('parent=-1'))).toBe(true)      // 根
    expect(labelSets.some(l => l.includes('孩子链 2→3'))).toBe(true)     // 结点 1 的孩子链
    expect(allNarration(steps)).toContain('n+1 = 6')
  })
  it('n=1 退化输入仍出完整三段；纯函数', () => {
    const steps = treeStorageViewsSteps({ values: [9] })
    expect(steps.length).toBeGreaterThan(3)
    expect(allNarration(steps)).toContain('孩子兄弟')
    expectPure(treeStorageViewsSteps, input)
  })
})

// ---------- 2. seq-bitree-index ----------
describe('seqBitreeIndex 生成器', () => {
  const input = { values: [1, 2, 3, 4, 5] }
  it('逐结点给双亲/左孩子/右孩子帧，narration 有公式代入（⌊3/2⌋=1、2×2=4、2×2+1=5）', () => {
    const steps = seqBitreeIndexSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('⌊3/2⌋ = 1')
    expect(all).toContain('2×2 = 4')
    expect(all).toContain('2×2+1 = 5')
    expect(all).toContain('根结点无双亲')
    expectTreeContract(steps)
  })
  it('nodeLabel 标 i、边标 2i/2i+1（结点 4 的 edgeLabel = 2×2=4）', () => {
    const steps = seqBitreeIndexSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(1).nodeLabel).toBe('i=1')
    expect(byId.get(4).edgeLabel).toBe('2×2=4')
    expect(byId.get(5).edgeLabel).toBe('2×2+1=5')
  })
  it('纯函数', () => { expectPure(seqBitreeIndexSteps, input) })
})

// ---------- 3. bitree-traverse（四模式共用一生成器） ----------
describe('bitreeTraverse 生成器', () => {
  const tokens = 'A,B,C,#,D,#,#,E,#,#,F,#,#'.split(',')
  it('pre 模式：访问序 A B C D E F，逐结点 done 高亮累计', () => {
    const steps = bitreeTraverseSteps({ mode: 'pre', tokens })
    expect(visitOrder(steps, '先序')).toEqual(['A', 'B', 'C', 'D', 'E', 'F'])
    const fin = last(steps)
    const doneCount = (fin.state as any).nodes.filter((n: any) => (n.keyHighlight ?? []).length).length
    expect(doneCount).toBe(6)
    expect(fin.narration).toContain('先序序 = A B C D E F')
    expectTreeContract(steps)
  })
  it('in / post / level 三模式访问序与理论一致，narration 给递归展开描述', () => {
    const inSteps = bitreeTraverseSteps({ mode: 'in', tokens })
    expect(visitOrder(inSteps, '中序')).toEqual(['C', 'D', 'B', 'E', 'A', 'F'])
    expect(allNarration(inSteps)).toContain('已归来')
    const postSteps = bitreeTraverseSteps({ mode: 'post', tokens })
    expect(visitOrder(postSteps, '后序')).toEqual(['D', 'C', 'E', 'B', 'F', 'A'])
    const levelSteps = bitreeTraverseSteps({ mode: 'level', tokens: 'A,B,F,C,E,D'.split(',') })
    expect(visitOrder(levelSteps, '层序')).toEqual(['A', 'B', 'F', 'C', 'E', 'D'])
    expect(allNarration(levelSteps)).toContain('入队')
    for (const s of [inSteps, postSteps, levelSteps]) expectTreeContract(s)
  })
  it('纯函数', () => {
    expectPure(bitreeTraverseSteps, { mode: 'pre' as const, tokens })
  })
})

// ---------- 4. thread-build ----------
describe('threadBuild 生成器', () => {
  const input = { tokens: 'A,B,C,#,D,#,#,E,#,#,F,#,#'.split(',') }
  it('终帧线索标签与中序 C D B E A F 一致（D 前驱→C 后继→B；F 后继→NULL）', () => {
    const steps = threadBuildSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(2).nodeLabel).toContain('前驱→NULL')   // C：中序首结点（id=2）
    expect(byId.get(3).nodeLabel).toContain('前驱→C')      // D（id=3）
    expect(byId.get(3).nodeLabel).toContain('后继→B')
    expect(byId.get(5).nodeLabel).toContain('后继→NULL')   // F：中序末结点（id=5）
    const all = allNarration(steps)
    expect(all).toContain('ltag=1')
    expect(all).toContain('rtag=1')
    expect(all).toContain('n+1 = 7 个空链域可存 n-1 = 5 条')
    expectTreeContract(steps)
  })
  it('线索不画成树边：任意帧的孩子数不因线索增加（B 保持二叉两孩子）', () => {
    const steps = threadBuildSteps(input)
    for (const s of steps) {
      const b = (s.state as any).nodes.find((n: any) => n.id === 1)
      if (b) expect(b.children.length).toBeLessThanOrEqual(2)
    }
  })
  it('纯函数', () => { expectPure(threadBuildSteps, input) })
})

// ---------- 5. forest-convert ----------
describe('forestConvert 生成器', () => {
  const input = { trees: [[1, 2, 3], [4, 5]] }
  it('终帧二叉形态：1 左 2 右 4；2 右 3；4 左 5', () => {
    const steps = forestConvertSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(1).children).toEqual([2, 4])
    expect(byId.get(2).children).toEqual([3])
    expect(byId.get(4).children).toEqual([5])
    expect(o.rootId).toBe(1)
    const all = allNarration(steps)
    expect(all).toContain('first_child → left')
    expect(all).toContain('next_sibling → right')
    expect(all).toContain('右链')
    expectTreeContract(steps)
  })
  it('首帧为森林（虚拟总根 keys 空、孩子 = 两棵树根），重连帧逐一 edgeActive', () => {
    const steps = forestConvertSteps(input)
    const first = steps[0].state as any
    const vroot = first.nodes.find((n: any) => n.id === 0)
    expect(vroot.keys).toEqual([])
    expect(vroot.children).toEqual([1, 4])
    expect(steps.some(s => (s.state as any).nodes.some((n: any) => (n.edgeActive ?? []).length > 0))).toBe(true)
  })
  it('纯函数', () => { expectPure(forestConvertSteps, input) })
})

// ---------- 6. huffman-build ----------
describe('huffmanBuild 生成器', () => {
  const input = { weights: [5, 2, 9, 1, 7] }
  it('终帧结构与 ht_build 一致：根 24 = 9+15；WPL = 44 逐叶累加', () => {
    const steps = huffmanBuildSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(9).keys).toEqual([24])
    expect(byId.get(9).children).toEqual([3, 8])   // node9: left=3(w9) right=8(w15)
    expect(byId.get(8).children).toEqual([5, 7])
    expect(byId.get(7).children).toEqual([6, 1])
    expect(byId.get(6).children).toEqual([4, 2])   // 小者为左：1 号位权 1 在左
    const fin = last(steps)
    expect(fin.narration).toContain('5×3 + 2×4 + 9×1 + 1×4 + 7×2 = 50')
    const all = allNarration(steps)
    expect(all).toContain('权 = 1 + 2 = 3')
    expect(all).toContain('平手取下标小者')
    expectTreeContract(steps)
  })
  it('合并轮数 = n-1 = 4，每轮先选最小两帧再合并一帧', () => {
    const steps = huffmanBuildSteps(input)
    const selects = steps.filter(s => s.narration.includes('轮选最小'))
    const merges = steps.filter(s => s.narration.includes('新结点 nodes['))
    expect(selects).toHaveLength(4)
    expect(merges).toHaveLength(4)
  })
  it('纯函数', () => { expectPure(huffmanBuildSteps, input) })
})

// ---------- 7. union-find-ops ----------
describe('unionFindOps 生成器', () => {
  const input = { ops: [
    { op: 'u', a: 1, b: 2 }, { op: 'u', a: 3, b: 4 },
    { op: 'f', a: 2 }, { op: 'u', a: 1, b: 3 }, { op: 'f', a: 4 }
  ], n: 4 }
  it('终帧：全并到根 1（2、3 直挂，4 经路径压缩直挂），narration 给 size 比较与压缩', () => {
    const steps = unionFindOpsSteps(input)
    const o = last(steps).state as any
    const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(2).children).toEqual([])
    expect(byId.get(3).children).toEqual([])   // 4 已从 3 直挂 1
    expect(byId.get(1).children).toEqual([2, 3, 4])
    const all = allNarration(steps)
    expect(all).toContain('4 的 parent 直接改为根 1（原来是 3）')
    expect(all).toContain('小树挂大树')
    expect(all).toContain('源码仅当 size[rx] < size[ry] 才交换')
    expect(last(steps).narration).toContain('剩 1 个集合')
    expectTreeContract(steps)
  })
  it('find 逐点改挂帧存在；同集 union 有 merged=0 帧', () => {
    const steps = unionFindOpsSteps({ ops: [
      { op: 'u', a: 1, b: 2 }, { op: 'u', a: 3, b: 4 }, { op: 'u', a: 2, b: 4 },
      { op: 'f', a: 4 }, { op: 'u', a: 1, b: 3 }
    ], n: 4 })
    const all = allNarration(steps)
    expect(all).toContain('4 的 parent 直接改为根 1（原来是 3）')
    expect(all).toContain('本就同集，无需合并')
    expectTreeContract(steps)
  })
  it('纯函数', () => { expectPure(unionFindOpsSteps, input) })
})

// ---------- 8. bst-insert-search ----------
describe('bstInsertSearch 生成器', () => {
  const input = { keys: [5, 2, 9, 1, 7], target: 7 }
  it('查找 7 的比较路径 5→9→7，narration 逐次比较；终帧命中绿色', () => {
    const steps = bstInsertSearchSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('7 vs 5：7 > 5 → 走右子树')
    expect(all).toContain('7 vs 9：7 < 9 → 走左子树')
    expect(all).toContain('查找成功！比较 3 次命中')
    const fin = last(steps)
    const hit = (fin.state as any).nodes.find((n: any) => n.id === 7)
    expect(hit.keyHighlight).toEqual([0])
    expect(fin.narration).toContain('中序遍历 = 升序序列 1, 2, 5, 7, 9')
    expectTreeContract(steps)
  })
  it('未命中：走至空指针报 DS_NOT_FOUND；插入帧交代"新结点必为叶子"', () => {
    const miss = bstInsertSearchSteps({ keys: [5, 2, 9, 1, 7], target: 8 })
    expect(last(miss).narration).toContain('DS_NOT_FOUND')
    const all = allNarration(miss)
    expect(all).toContain('8 vs 7：8 > 7 → 走右子树')
    expect(all).toContain('挂新叶子')
    expectTreeContract(miss)
  })
  it('纯函数', () => { expectPure(bstInsertSearchSteps, input) })
})

// ---------- 9. avl-rotate ----------
describe('avlRotate 生成器', () => {
  it('默认 1,2,3 触发 RR：旋转前后两帧换根 1→2，终帧根 2 带孩子 1、3', () => {
    const steps = avlRotateSteps({ keys: [1, 2, 3] })
    const all = allNarration(steps)
    expect(all).toContain('RR 形态，直接左单旋')
    expect(all).toContain('rot_left 完成：2 升为子树根，1 挂到 2 左侧')
    expect(all).toContain('最小失衡子树根')
    const fin = last(steps).state as any
    const byId = new Map(fin.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(2).children).toEqual([1, 3])
    expectTreeContract(steps)
  })
  it('LL / LR / RL 三种形态各得新根 2，narration 点明类型', () => {
    const ll = avlRotateSteps({ keys: [3, 2, 1] })
    expect(allNarration(ll)).toContain('LL 形态，直接右单旋')
    expect(new Map((last(ll).state as any).nodes.map((n: any) => [n.id, n])).get(2).children).toEqual([1, 3])
    const lr = avlRotateSteps({ keys: [3, 1, 2] })
    expect(allNarration(lr)).toContain('LR 折线')
    expect(allNarration(lr)).toContain('第一次 rot_left')
    expect(new Map((last(lr).state as any).nodes.map((n: any) => [n.id, n])).get(2).children).toEqual([1, 3])
    const rl = avlRotateSteps({ keys: [1, 3, 2] })
    expect(allNarration(rl)).toContain('RL 折线')
    expect(allNarration(rl)).toContain('第一次 rot_right')
    expect(new Map((last(rl).state as any).nodes.map((n: any) => [n.id, n])).get(2).children).toEqual([1, 3])
    for (const s of [ll, lr, rl]) expectTreeContract(s)
  })
  it('nodeLabel 标 h/BF；5 键序列 5,3,8,2,1 触发 LL 旋转', () => {
    const steps = avlRotateSteps({ keys: [5, 3, 8, 2, 1] })
    const framesWithBF = steps.filter(s => (s.state as any).nodes.some((n: any) => /BF=-?\d/.test(n.nodeLabel ?? '')))
    expect(framesWithBF.length).toBeGreaterThan(0)
    expectTreeContract(steps)
  })
  it('纯函数', () => { expectPure(avlRotateSteps, { keys: [1, 2, 3] }) })
})

// ---------- 10. rb-insert ----------
describe('rbInsert 生成器', () => {
  it('10,5,15,3,1：叔红变色上推 + 叔黑 LL 右旋，终帧结构与 rb.c 一致', () => {
    const steps = rbInsertSteps({ keys: [10, 5, 15, 3, 1] })
    const all = allNarration(steps)
    expect(all).toContain('叔叔红 → 变色上推：父 5 与叔 15 变黑、祖父 10 变红')
    expect(all).toContain('LL 直线：父 3 染黑、祖父 5 染红、右旋 5')
    const fin = last(steps).state as any
    const byId = new Map(fin.nodes.map((n: any) => [n.id, n]))
    expect(fin.rootId).toBe(10)
    expect(byId.get(10).color).toBe('B')
    expect(byId.get(3).color).toBe('B')
    expect(byId.get(3).children).toEqual([1, 5])
    expect(byId.get(1).color).toBe('R')
    expect(byId.get(5).color).toBe('R')
    expect(byId.get(15).color).toBe('B')
    expect(all).toContain('根恒黑（性质2）')
    expectTreeContract(steps)
  })
  it('10,5,20,3：纯变色情形（叔红上推到根染黑），无旋转帧', () => {
    const steps = rbInsertSteps({ keys: [10, 5, 20, 3] })
    const all = allNarration(steps)
    expect(all).toContain('叔叔红 → 变色上推')
    expect(all).not.toContain('折线')
    const fin = last(steps).state as any
    const byId = new Map(fin.nodes.map((n: any) => [n.id, n]))
    expect(byId.get(3).color).toBe('R')
    expect(byId.get(10).color).toBe('B')
    expectTreeContract(steps)
  })
  it('纯函数', () => { expectPure(rbInsertSteps, { keys: [10, 5, 15, 3, 1] }) })
})

// ---------- 11. btree-split ----------
describe('btreeSplit 生成器', () => {
  const input = { keys: [10, 20, 40, 50, 30, 60] }
  it('t=3 语义：第 6 键触发根分裂，中键 30 上移；终帧根 [30] 带两孩子', () => {
    const steps = btreeSplitSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('根满（[10, 20, 30, 40, 50]，5 = 2t−1 键）')
    expect(all).toContain('根分裂：新根')
    expect(all).toContain('收中键 30')
    expect(all).toContain('左 [10, 20] / 右 [40, 50]')
    const fin = last(steps).state as any
    const byId = new Map(fin.nodes.map((n: any) => [n.id, n]))
    const root = fin.nodes.find((n: any) => n.id === fin.rootId)
    expect(root.keys).toEqual([30])
    expect(root.children).toHaveLength(2)
    const kidKeys = root.children.map((c: number) => (byId.get(c) as any).keys)
    expect(kidKeys).toEqual([[10, 20], [40, 50, 60]])
    expect(last(steps).narration).toContain('中序遍历 = 10, 20, 30, 40, 50, 60')
    expectTreeContract(steps)
  })
  it('分裂帧先给满结点高亮（先分裂再下降），叶插入帧给有序落位', () => {
    const steps = btreeSplitSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('先分裂再下降')
    expect(all).toContain('叶子有序插入 30')
    expectTreeContract(steps)
  })
  it('纯函数', () => { expectPure(btreeSplitSteps, input) })
})

// ---------- 12. bplus-insert ----------
describe('bplusInsert 生成器', () => {
  const input = { keys: [10, 20, 30, 40, 50, 60, 70, 80, 90, 15] }
  it('两次叶分裂：路由键 40、70 上浮（复制非移动）；终帧根 [40,70] 三叶', () => {
    const steps = bplusInsertSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('根分裂收尾：新根')
    expect(all).toContain('路由键 70 上浮插入内部')
    expect(all).toContain('"复制"上浮为路由键')
    const fin = last(steps).state as any
    const root = fin.nodes.find((n: any) => n.id === fin.rootId)
    expect(root.keys).toEqual([40, 70])
    expect(root.children).toHaveLength(3)
    const byId = new Map(fin.nodes.map((n: any) => [n.id, n]))
    const leafKeys = root.children.map((c: number) => (byId.get(c) as any).keys)
    expect(leafKeys).toEqual([[10, 15, 20, 30], [40, 50, 60], [70, 80, 90]])
    expect(last(steps).state.note).toContain('叶链：[10,15,20,30] → [40,50,60] → [70,80,90]')
    expect(all).toContain('右子树最小键')
    expectTreeContract(steps)
  })
  it('等值路由走右、普通插入不改路由键的说明出现', () => {
    const steps = bplusInsertSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('普通插入不更新路由键')
    expectTreeContract(steps)
  })
  it('纯函数', () => { expectPure(bplusInsertSteps, input) })
})

// ---------- registry：批3 新增键 ----------
const B3_KEYS = [
  'tree-storage-views', 'seq-bitree-index', 'bitree-traverse', 'thread-build',
  'forest-convert', 'huffman-build', 'union-find-ops', 'bst-insert-search',
  'avl-rotate', 'rb-insert', 'btree-split', 'bplus-insert'
]

describe('registry 批3 新增键', () => {
  it('12 个新键全部注册、renderer 均为 tree、defaultInput 通过自身校验', () => {
    for (const k of B3_KEYS) {
      const def = registry[k]
      expect(def, `缺键 ${k}`).toBeTruthy()
      expect(def.renderer, `${k} renderer`).toBe('tree')
      expect(def.validate(def.parse(def.defaultInput))).toBeNull()
    }
    expect(Object.keys(registry)).toHaveLength(47)
  })
  it('全链路：defaultInput 经 parse→validate→steps 产出有效帧（PlayerShell 同款调用序）', () => {
    for (const k of B3_KEYS) {
      const def = registry[k]
      const parsed = def.parse(def.defaultInput)
      const steps = def.steps(parsed)
      expect(steps.length, `${k} 帧数不足`).toBeGreaterThan(1)
      for (const s of steps) expect(s.narration.length, `${k} 空 narration`).toBeGreaterThan(0)
    }
  })
  it('非法输入返回中文提示（含示例字样"如"）', () => {
    expect(registry['tree-storage-views'].validate(registry['tree-storage-views'].parse('1,2,2,3,4'))).toContain('不可重复')
    expect(registry['seq-bitree-index'].validate(registry['seq-bitree-index'].parse('1,2,3,4,5,6,7,8'))).toContain('如')
    expect(registry['bitree-traverse'].validate(registry['bitree-traverse'].parse('mid:A,B,#'))).toContain('模式前缀')
    expect(registry['bitree-traverse'].validate(registry['bitree-traverse'].parse('level:A,#,B'))).toContain('不带空标记')
    expect(registry['bitree-traverse'].validate(registry['bitree-traverse'].parse('pre:#'))).toContain('构不成树')
    expect(registry['forest-convert'].validate(registry['forest-convert'].parse('1,2,3;4,5;6,1'))).toContain('不可重复')
    expect(registry['huffman-build'].validate(registry['huffman-build'].parse('5,0,2'))).toContain('正数')
    expect(registry['union-find-ops'].validate(registry['union-find-ops'].parse('u:1,2,x:9'))).toContain('看不懂')
    expect(registry['bst-insert-search'].validate(registry['bst-insert-search'].parse('5,2|7,9'))).toContain('查找目标')
    expect(registry['avl-rotate'].validate(registry['avl-rotate'].parse('5,2,9,1'))).toContain('1,2,3 触发 RR')
    expect(registry['rb-insert'].validate(registry['rb-insert'].parse('10,5,20'))).toContain('4~8 个键')
    expect(registry['btree-split'].validate(registry['btree-split'].parse('10,20,40'))).toContain('6~8 个键')
    expect(registry['bplus-insert'].validate(registry['bplus-insert'].parse('10,20,30,40,50,10'))).toContain('不可重复')
  })
})
