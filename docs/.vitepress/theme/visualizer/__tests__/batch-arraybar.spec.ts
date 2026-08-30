// docs/.vitepress/theme/visualizer/__tests__/batch-arraybar.spec.ts
// 批1：ArrayBar 复用扩展的 16 个动画（线性表/矩阵/图建图/查找/散列）
import { describe, it, expect } from 'vitest'
import type { Step } from '../types'
import { expectFrameInvariants } from './steps.common'
import { registry } from '../registry'

import { seqListInsertSteps } from '../steps/seqListInsert'
import { seqListRemoveSteps } from '../steps/seqListRemove'
import { dynaSeqGrowSteps } from '../steps/dynaSeqGrow'
import { staticListCursorSteps } from '../steps/staticListCursor'
import { symmetricCompressSteps } from '../steps/symmetricCompress'
import { triangularCompressSteps } from '../steps/triangularCompress'
import { diagonalCompressSteps } from '../steps/diagonalCompress'
import { tripleTransposeSteps } from '../steps/tripleTranspose'
import { seqSearchSteps } from '../steps/seqSearch'
import { binSearchSteps } from '../steps/binSearch'
import { blockSearchSteps } from '../steps/blockSearch'
import { hashFuncMapSteps } from '../steps/hashFuncMap'
import { hashOpenProbeSteps } from '../steps/hashOpenProbe'
import { hashAslCompareSteps } from '../steps/hashAslCompare'
import { graphMatrixBuildSteps } from '../steps/graphMatrixBuild'
import { hashChainInsertSteps } from '../steps/hashChainInsert'

const clone = <T,>(x: T): T => structuredClone(x)
const last = (steps: Step[]): Step => steps[steps.length - 1]

/** 桶形态 state 的通用帧不变量：narration 非空、shape 正确、高亮不越 array 界 */
function expectBucketFrames(steps: Step[], arrLen?: (s: Step) => number) {
  expect(steps.length).toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length).toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.array)).toBe(true)
    expect(Array.isArray(o.buckets)).toBe(true)
    expect(Array.isArray(o.bucketLabels)).toBe(true)
    expect(o.buckets.length).toBe(o.bucketLabels.length)
    const len = arrLen ? arrLen(s) : (o.array as number[]).length
    for (const i of [...s.highlights, ...(s.active ?? [])]) {
      expect(i).toBeGreaterThanOrEqual(0)
      expect(i).toBeLessThan(len)
    }
  }
}

/** 纯函数：同输入两次运行结果一致，且不改输入 */
function expectPure<T>(fn: (x: T) => Step[], input: T) {
  const before = clone(input)
  const a = fn(input)
  const b = fn(input)
  expect(input).toEqual(before)   // 不修改输入
  expect(a).toEqual(b)            // 输出确定性
}

// ---------- 1. seqlist-insert 静态顺序表插入 ----------
describe('seqListInsert 生成器', () => {
  const input = { list: [12, 25, 33, 47, 58], pos: 3, value: 40 }
  it('最终帧：第 3 位插入 40，尾部空槽为 0，narration 含完成', () => {
    const steps = seqListInsertSteps(input)
    expect(last(steps).state).toEqual([12, 25, 40, 33, 47, 58, 0, 0])
    expect(last(steps).narration).toContain('完成')
    expectFrameInvariants(steps, 8)
  })
  it('表尾追加（pos = length+1）不触发后移，直接落位', () => {
    const steps = seqListInsertSteps({ list: [12, 25], pos: 3, value: 99 })
    expect(last(steps).state).toEqual([12, 25, 99, 0, 0, 0, 0, 0])
    expect(steps.some(s => s.narration.includes('后移'))).toBe(false)
  })
  it('首位插入全部后移；中间帧 narration 给出下标推理', () => {
    const steps = seqListInsertSteps({ list: [12, 25], pos: 1, value: 7 })
    expect(last(steps).state).toEqual([7, 12, 25, 0, 0, 0, 0, 0])
    expect(steps.some(s => s.narration.includes('data['))).toBe(true)
  })
  it('纯函数', () => { expectPure(seqListInsertSteps, input) })
})

// ---------- 2. seqlist-remove 静态顺序表删除 ----------
describe('seqListRemove 生成器', () => {
  const input = { list: [12, 25, 33, 47, 58], pos: 2 }
  it('最终帧：第 2 位元素被删，其余前移，尾部空槽为 0', () => {
    const steps = seqListRemoveSteps(input)
    expect(last(steps).state).toEqual([12, 33, 47, 58, 0, 0, 0, 0])
    expect(last(steps).narration).toContain('完成')
    expectFrameInvariants(steps, 8)
  })
  it('中间帧 narration 提到前移与被删值', () => {
    const steps = seqListRemoveSteps(input)
    expect(steps.some(s => s.narration.includes('前移'))).toBe(true)
    expect(steps.some(s => s.narration.includes('25'))).toBe(true)
  })
  it('纯函数', () => { expectPure(seqListRemoveSteps, input) })
})

// ---------- 3. dynaseq-grow 动态顺序表扩容搬家 ----------
describe('dynaSeqGrow 生成器', () => {
  const input = [5, 12, 18, 23]
  it('state 为桶形态；最终帧新块装下全部元素，旧桶标记已 free', () => {
    const steps = dynaSeqGrowSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.array.slice(0, 4)).toEqual([5, 12, 18, 23])
    expect(o.array).toHaveLength(8)                 // 双倍新数组
    expect(o.buckets[0]).toEqual([])                // 旧表已释放
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps)
  })
  it('narration 依次出现容量满、新数组、逐个搬运、free', () => {
    const steps = dynaSeqGrowSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('满')
    expect(all).toContain('2×4 = 8')
    expect(all).toContain('new_data[3] = old_data[3]')
    expect(all).toContain('free')
  })
  it('纯函数', () => { expectPure(dynaSeqGrowSteps, input) })
})

// ---------- 4. staticlist-cursor 静态链表游标插入 ----------
describe('staticListCursor 生成器', () => {
  const input = { list: [10, 20, 30, 40], pos: 3, value: 25 }
  it('state 为游标数组（长度 10）；插入后沿头结点走链含新分量', () => {
    const steps = staticListCursorSteps(input)
    const fin = last(steps)
    const cur = fin.state as number[]
    expect(cur).toHaveLength(10)
    // 从头结点 9 沿 cur 走数据链：1→2→5(新)→3→4
    const chain: number[] = []
    let p = cur[9]
    while (p !== 0 && chain.length < 10) { chain.push(p); p = cur[p] }
    expect(chain).toEqual([1, 2, 5, 3, 4])
    expect(cur[0]).toBe(6)                          // 备用链头让位
    expect(fin.narration).toContain('完成')
    expectFrameInvariants(steps, 10)
  })
  it('中间帧：沿游标寻位与 malloc 分量都有交代', () => {
    const steps = staticListCursorSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('沿 cur 走')
    expect(all).toContain('空闲分量 5')
  })
  it('纯函数', () => { expectPure(staticListCursorSteps, input) })
})

// ---------- 5. symmetric-compress 对称矩阵压缩 ----------
describe('symmetricCompress 生成器', () => {
  const input = [1, 2, 4, 3, 5, 7, 6, 8, 9, 10]   // 4 阶下三角
  it('最终帧压缩数组 = 输入，桶按行分组长度 1..4', () => {
    const steps = symmetricCompressSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.array).toEqual(input)
    expect(o.buckets.map((b: number[]) => b.length)).toEqual([1, 2, 3, 4])
    expect(o.buckets.flat()).toEqual(input)
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps)
  })
  it('每个放置帧 narration 给出 k = i(i-1)/2+j-1 的算式', () => {
    const steps = symmetricCompressSteps(input)
    const placed = steps.filter(s => s.narration.includes('k ='))
    expect(placed.length).toBe(10)
    expect(placed.some(s => s.narration.includes('a(3,2) = 5：k = 3×2/2 + 2 − 1 = 4'))).toBe(true)
  })
  it('纯函数', () => { expectPure(symmetricCompressSteps, input) })
})

// ---------- 6. triangular-compress 三角矩阵压缩 ----------
describe('triangularCompress 生成器', () => {
  const input = { values: [1, 2, 4, 3, 5, 7], c: 9 } // 3 阶下三角 + 常数
  it('最终帧 = 三角区元素 + 末位常数 c，桶含常数桶', () => {
    const steps = triangularCompressSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.array).toEqual([1, 2, 4, 3, 5, 7, 9])
    expect(o.bucketLabels[o.bucketLabels.length - 1]).toContain('常数')
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps)
  })
  it('narration 给出 k 算式与常数 c 的带外交代', () => {
    const steps = triangularCompressSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('k = 3×2/2 + 2 − 1 = 4')
    expect(all).toContain('c = 9')
  })
  it('纯函数', () => { expectPure(triangularCompressSteps, input) })
})

// ---------- 7. diagonal-compress 三对角矩阵压缩 ----------
describe('diagonalCompress 生成器', () => {
  const input = [4, 7, 3, 9, 5, 8, 6]             // 3 阶三条对角线按行序
  it('最终帧压缩数组 = 输入，桶按行分组（2/3/2）', () => {
    const steps = diagonalCompressSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.array).toEqual(input)
    expect(o.buckets.map((b: number[]) => b.length)).toEqual([2, 3, 2])
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps)
  })
  it('narration 给出 k = 2i+j-3 算式', () => {
    const steps = diagonalCompressSteps(input)
    expect(steps.some(s => s.narration.includes('a(2,3) = 5：k = 2×2 + 3 − 3 = 4'))).toBe(true)
  })
  it('纯函数', () => { expectPure(diagonalCompressSteps, input) })
})

// ---------- 8. triple-transpose 快速转置 ----------
describe('tripleTranspose 生成器', () => {
  const input = {
    triples: [{ r: 1, c: 2, v: 12 }, { r: 2, c: 2, v: 9 }, { r: 3, c: 1, v: 5 }, { r: 3, c: 4, v: 7 }],
    cols: 4
  }
  it('转置表按新行序落位：[5,12,9,7]；buckets 为 num/cpot 两组', () => {
    const steps = tripleTransposeSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.array).toEqual([5, 12, 9, 7])
    expect(o.bucketLabels).toEqual(['num（各列个数）', 'cpot（列首元落位）'])
    expect(o.buckets[0]).toEqual([1, 2, 0, 1])      // num
    expect(o.buckets[1]).toEqual([1, 3, 3, 4])      // cpot 落位后（各列游标已推进）
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps)
  })
  it('narration 给出 cpot 递推式', () => {
    const steps = tripleTransposeSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('cpot[1] = cpot[0] + num[0] = 0 + 1 = 1')
    expect(all).toContain('cpot[2] = cpot[1] + num[1] = 1 + 2 = 3')
  })
  it('纯函数', () => { expectPure(tripleTransposeSteps, input) })
})

// ---------- 9. seq-search 哨兵顺序查找 ----------
describe('seqSearch 生成器', () => {
  it('命中：末位是哨兵 key，highlights 指命中位，narration 含命中', () => {
    const steps = seqSearchSteps({ a: [3, 7, 1, 9, 5], key: 9 })
    const fin = last(steps)
    expect(fin.state).toEqual([3, 7, 1, 9, 5, 9])
    expect(fin.highlights).toEqual([3])
    expect(fin.narration).toContain('命中')
    expectFrameInvariants(steps, 6)
  })
  it('未命中：一路比到哨兵，narration 含失败', () => {
    const steps = seqSearchSteps({ a: [3, 7, 1], key: 8 })
    const fin = last(steps)
    expect(fin.state).toEqual([3, 7, 1, 8])
    expect(fin.highlights).toEqual([])
    expect(fin.narration).toContain('失败')
  })
  it('首帧交代哨兵埋设', () => {
    const steps = seqSearchSteps({ a: [3, 7, 1], key: 8 })
    expect(steps[0].narration).toContain('哨兵')
  })
  it('纯函数', () => { expectPure(seqSearchSteps, { a: [3, 7, 1, 9, 5], key: 9 }) })
})

// ---------- 10. bin-search 折半查找 ----------
describe('binSearch 生成器', () => {
  const input = { a: [1, 3, 5, 7, 9, 11], key: 7 }
  it('命中：mid 三点高亮推进，最终 highlights=[3]', () => {
    const steps = binSearchSteps(input)
    const fin = last(steps)
    expect(fin.state).toEqual([1, 3, 5, 7, 9, 11])
    expect(fin.highlights).toEqual([3])
    expect(fin.narration).toContain('命中')
    expectFrameInvariants(steps, 6)
    expect(steps.some(s => s.narration.includes('mid = 2'))).toBe(true)
  })
  it('未命中：区间缩空，narration 含未找到', () => {
    const steps = binSearchSteps({ a: [1, 3, 5, 7, 9, 11], key: 6 })
    expect(last(steps).highlights).toEqual([])
    expect(last(steps).narration).toContain('未找到')
  })
  it('每轮帧 active=[low,high] 且 highlights=[mid]', () => {
    const steps = binSearchSteps(input)
    const rounds = steps.filter(s => s.narration.includes('mid ='))
    expect(rounds.length).toBeGreaterThan(0)
    for (const s of rounds) {
      expect(s.active).toHaveLength(2)
      expect(s.highlights).toHaveLength(1)
    }
  })
  it('纯函数', () => { expectPure(binSearchSteps, input) })
})

// ---------- 11. block-search 分块查找 ----------
describe('blockSearch 生成器', () => {
  const input = { a: [5, 8, 3, 12, 15, 22, 18, 25, 30, 27, 42, 36], blockSize: 4, key: 25 }
  it('state 桶形态：3 块、标签带 max_key；命中块 2 下标 7', () => {
    const steps = blockSearchSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.buckets.map((b: number[]) => b)).toEqual([[5, 8, 3, 12], [15, 22, 18, 25], [30, 27, 42, 36]])
    expect(o.bucketLabels[0]).toContain('max=12')
    expect(o.bucketLabels[1]).toContain('max=25')
    expect(fin.highlights).toEqual([7])
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps, () => 12)
  })
  it('未命中：索引定块后块内扫空，narration 含未找到', () => {
    const steps = blockSearchSteps({ a: [5, 8, 3, 12, 15, 22, 18, 25, 30, 27, 42, 36], blockSize: 4, key: 19 })
    expect(last(steps).highlights).toEqual([])
    expect(last(steps).narration).toContain('未找到')
  })
  it('narration 交代索引表定块两阶段', () => {
    const steps = blockSearchSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('max_key')
    expect(all).toContain('块内')
  })
  it('纯函数', () => { expectPure(blockSearchSteps, input) })
})

// ---------- 12. hash-func-map 除留余数映射 ----------
describe('hashFuncMap 生成器', () => {
  const input = { keys: [12, 39, 18, 24, 33, 21], p: 7 }
  it('p 个桶；最终帧各键按 key%p 归桶', () => {
    const steps = hashFuncMapSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.bucketLabels).toHaveLength(7)
    expect(o.buckets[0]).toEqual([21])
    expect(o.buckets[3]).toEqual([24])
    expect(o.buckets[4]).toEqual([39, 18])
    expect(o.buckets[5]).toEqual([12, 33])
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps, () => 6)
  })
  it('每个关键字的 narration 给出 H(key) = key % p 算式', () => {
    const steps = hashFuncMapSteps(input)
    expect(steps.some(s => s.narration.includes('H(39) = 39 % 7 = 4'))).toBe(true)
  })
  it('纯函数', () => { expectPure(hashFuncMapSteps, input) })
})

// ---------- 13. hash-open-probe 二次探测插入 ----------
describe('hashOpenProbe 生成器', () => {
  const input = { keys: [18, 24, 39, 60, 45, 12], p: 7 }
  it('冲突按 +1²,−1²,+2² 交替探测：60 从 4 → 5 → 3 → 1', () => {
    const steps = hashOpenProbeSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.buckets[1]).toEqual([60])
    expect(o.buckets[2]).toEqual([45])
    expect(o.buckets[3]).toEqual([24])
    expect(o.buckets[4]).toEqual([18])
    expect(o.buckets[5]).toEqual([39])
    expect(o.buckets[6]).toEqual([12])
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps, () => 6)
  })
  it('narration 给出 H_i 算式与落位', () => {
    const steps = hashOpenProbeSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('H₀(39) = 39 % 7 = 4')
    expect(all).toContain('落位')
  })
  it('纯函数', () => { expectPure(hashOpenProbeSteps, input) })
})

// ---------- 14. hash-asl-compare ASL 成功/失败对比 ----------
describe('hashAslCompare 生成器', () => {
  const input = [12, 39, 18, 24, 33, 21]          // n=6 → p=5
  it('桶按除留余数+头插成形，最终帧给出两个 ASL 分数', () => {
    const steps = hashAslCompareSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.buckets[1]).toEqual([21])
    expect(o.buckets[2]).toEqual([12])
    expect(o.buckets[3]).toEqual([33, 18])
    expect(o.buckets[4]).toEqual([24, 39])
    expect(fin.narration).toContain('完成')
    expect(fin.narration).toContain('8/6')
    expect(fin.narration).toContain('6/5')
    expectBucketFrames(steps, () => 6)
  })
  it('过程帧给出累计 ASL 分数', () => {
    const steps = hashAslCompareSteps(input)
    expect(steps.some(s => s.narration.includes('ASL成功'))).toBe(true)
    expect(steps.some(s => s.narration.includes('ASL失败'))).toBe(true)
  })
  it('纯函数', () => { expectPure(hashAslCompareSteps, input) })
})

// ---------- 15. graph-matrix-build 邻接矩阵建图 ----------
describe('graphMatrixBuild 生成器', () => {
  it('无向图（默认）：对称写 1', () => {
    const input = { edges: [{ u: 0, v: 1 }, { u: 1, v: 2 }, { u: 2, v: 3 }], n: 4, directed: false }
    const steps = graphMatrixBuildSteps(input)
    const o = last(steps).state as any
    expect(o.buckets).toEqual([[0, 1, 0, 0], [1, 0, 1, 0], [0, 1, 0, 1], [0, 0, 1, 0]])
    expect(o.bucketLabels).toEqual(['v0', 'v1', 'v2', 'v3'])
    expect(last(steps).narration).toContain('完成')
    expectBucketFrames(steps, () => 0)
  })
  it('有向图只写一处', () => {
    const input = { edges: [{ u: 0, v: 1 }, { u: 1, v: 2 }], n: 3, directed: true }
    const steps = graphMatrixBuildSteps(input)
    const o = last(steps).state as any
    expect(o.buckets).toEqual([[0, 1, 0], [0, 0, 1], [0, 0, 0]])
    expect(last(steps).narration).toContain('完成')
  })
  it('narration 给出 matrix[u][v]=1 的算式', () => {
    const input = { edges: [{ u: 0, v: 1 }], n: 2, directed: false }
    const steps = graphMatrixBuildSteps(input)
    expect(steps.some(s => s.narration.includes('matrix[0][1] = 1'))).toBe(true)
    expect(steps.some(s => s.narration.includes('matrix[1][0] = 1'))).toBe(true)
  })
  it('纯函数', () => {
    expectPure(graphMatrixBuildSteps, { edges: [{ u: 0, v: 1 }], n: 2, directed: false })
  })
})

// ---------- 16. hash-chain-insert 拉链法插入 ----------
describe('hashChainInsert 生成器', () => {
  const input = { keys: [12, 39, 18, 24, 33, 21], p: 7 }
  it('桶内链序为头插序：39 先于 18，12 先于 33', () => {
    const steps = hashChainInsertSteps(input)
    const fin = last(steps)
    const o = fin.state as any
    expect(o.buckets[4]).toEqual([18, 39])
    expect(o.buckets[5]).toEqual([33, 12])
    expect(o.buckets[0]).toEqual([21])
    expect(fin.narration).toContain('完成')
    expectBucketFrames(steps, () => 6)
  })
  it('narration 给出 H(key) 与链位置', () => {
    const steps = hashChainInsertSteps(input)
    const all = steps.map(s => s.narration).join('\n')
    expect(all).toContain('H(18) = 18 % 7 = 4')
    expect(all).toContain('头插')
  })
  it('纯函数', () => { expectPure(hashChainInsertSteps, input) })
})

// ---------- registry：批1 新增键的 parse/validate ----------
const NEW_KEYS = [
  'seqlist-insert', 'seqlist-remove', 'dynaseq-grow', 'staticlist-cursor',
  'symmetric-compress', 'triangular-compress', 'diagonal-compress', 'triple-transpose',
  'seq-search', 'bin-search', 'block-search', 'hash-func-map',
  'hash-open-probe', 'hash-asl-compare', 'graph-matrix-build', 'hash-chain-insert'
]

describe('registry 批1 新增键', () => {
  it('16 个新键全部注册且 defaultInput 通过自身校验', () => {
    for (const k of NEW_KEYS) {
      const def = registry[k]
      expect(def, `缺键 ${k}`).toBeTruthy()
      expect(def.validate(def.parse(def.defaultInput))).toBeNull()
    }
    expect(Object.keys(registry)).toHaveLength(25)
  })
  it('全链路：defaultInput 经 parse→validate→steps 产出有效帧（防 parse/steps 形状脱节）', () => {
    for (const k of NEW_KEYS) {
      const def = registry[k]
      const parsed = def.parse(def.defaultInput)
      const steps = def.steps(parsed)   // PlayerShell 同款调用序：validate 通过才走到这
      expect(steps.length, `${k} 帧数不足`).toBeGreaterThan(1)
      for (const s of steps) expect(s.narration.length, `${k} 空 narration`).toBeGreaterThan(0)
    }
  })
  it('非法输入返回中文提示（含示例字样）', () => {
    expect(registry['seqlist-insert'].validate(registry['seqlist-insert'].parse('12,25|9,7'))).toContain('如')
    expect(registry['bin-search'].validate(registry['bin-search'].parse('3,1,2|5'))).toContain('升序')
    expect(registry['hash-open-probe'].validate(registry['hash-open-probe'].parse('1,2|4'))).toContain('素数')
    expect(registry['block-search'].validate(registry['block-search'].parse('9,1,5,3|2|1'))).toContain('块间')
    expect(registry['graph-matrix-build'].validate(registry['graph-matrix-build'].parse('0-1,0-0|2'))).toContain('顶点')
  })
})
