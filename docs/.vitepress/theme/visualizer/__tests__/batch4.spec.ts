// docs/.vitepress/theme/visualizer/__tests__/batch4.spec.ts
// 批4：String / Graph 双渲染器与串、图、外排 15 个动画
// （定长串赋值/堆串扩容/块链/BF/KMP + 邻接表/十字链表/邻接多重表/Prim/Kruskal/Dijkstra/拓扑/关键路径/稀疏矩阵十字链表 + 外部归并）
import { describe, it, expect } from 'vitest'
import type { Step } from '../types'
import { registry } from '../registry'

import { stringStaticAssignSteps } from '../steps/stringStaticAssign'
import { heapStringGrowSteps } from '../steps/heapStringGrow'
import { blockStringChunkSteps } from '../steps/blockStringChunk'
import { bfMatchSteps } from '../steps/bfMatch'
import { kmpMatchSteps } from '../steps/kmpMatch'
import { adjlistBuildSteps } from '../steps/adjlistBuild'
import { orthlistBuildSteps } from '../steps/orthlistBuild'
import { amlBuildSteps } from '../steps/amlBuild'
import { mstPrimSteps } from '../steps/mstPrim'
import { mstKruskalSteps } from '../steps/mstKruskal'
import { spDijkstraSteps } from '../steps/spDijkstra'
import { topoKahnSteps } from '../steps/topoKahn'
import { externalMergeSteps } from '../steps/externalMerge'
import { cpCriticalPathSteps } from '../steps/cpCriticalPath'
import { crosslistSparseSteps } from '../steps/crosslistSparse'

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

/** String state 契约：rows 数组、label/chars 形状、capacity ≥ 长度、
 *  指针 index ∈ [-1, len]、字符高亮下标不越界、narration 非空（minFrames 允许失败单帧路径） */
function expectStringContract(steps: Step[], minFrames = 2) {
  expect(steps.length, '帧数不足').toBeGreaterThanOrEqual(minFrames)
  for (const s of steps) {
    expect(s.narration.length, 'narration 非空').toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.rows)).toBe(true)
    for (const r of o.rows) {
      expect(typeof r.label).toBe('string')
      expect(Array.isArray(r.chars)).toBe(true)
      const len = r.chars.length
      if (r.capacity !== undefined) expect(r.capacity, 'capacity 小于实际字符数').toBeGreaterThanOrEqual(len)
      for (const p of r.pointers ?? []) {
        expect(typeof p.name).toBe('string')
        expect(p.index).toBeGreaterThanOrEqual(-1)
        expect(p.index, `指针 ${p.name} 越界`).toBeLessThanOrEqual(len)
      }
      for (const k of [...(r.charHighlight ?? []), ...(r.charActive ?? []), ...(r.strike ?? [])]) {
        expect(k).toBeGreaterThanOrEqual(0)
        expect(k, '字符高亮越界').toBeLessThan(len)
      }
    }
    for (const a of o.note ? [o.note] : []) expect(typeof a).toBe('string')
  }
}

/** Graph state 契约：id 唯一、坐标 ∈ [0,100]、edges 的 from/to 必须引用存在的 id、
 *  aux/note 为文本、narration 非空（minFrames 允许失败单帧路径） */
function expectGraphContract(steps: Step[], minFrames = 2) {
  expect(steps.length, '帧数不足').toBeGreaterThanOrEqual(minFrames)
  for (const s of steps) {
    expect(s.narration.length, 'narration 非空').toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.nodes)).toBe(true)
    expect(Array.isArray(o.edges)).toBe(true)
    const ids = new Set<number>(o.nodes.map((n: any) => n.id))
    expect(ids.size).toBe(o.nodes.length)
    for (const nd of o.nodes) {
      expect(typeof nd.label).toBe('string')
      for (const c of [nd.x, nd.y]) {
        expect(typeof c).toBe('number')
        expect(c).toBeGreaterThanOrEqual(0)
        expect(c, `结点 ${nd.id} 坐标越出 0~100`).toBeLessThanOrEqual(100)
      }
    }
    for (const e of o.edges) {
      expect(ids.has(e.from), `边 from=${e.from} 悬空`).toBe(true)
      expect(ids.has(e.to), `边 to=${e.to} 悬空`).toBe(true)
    }
    for (const a of o.aux ?? []) expect(typeof a.text).toBe('string')
  }
}

// ---------- 1. string-static-assign ----------
describe('stringStaticAssign 生成器', () => {
  const input = { src: 'ababcabc' }
  it('逐字符拷入：终帧 data 与源串一致、length 递增 narration 带下标代入', () => {
    const steps = stringStaticAssignSteps(input)
    const o = last(steps).state as any
    expect(o.rows[1].chars).toEqual(['a', 'b', 'a', 'b', 'c', 'a', 'b', 'c'])
    expect(o.rows[1].capacity).toBe(12)
    expect(o.rows[1].label).toContain('length=8')
    expect(allNarration(steps)).toContain("s->data[3] = cstr[3] = 'b'")
    expect(last(steps).narration).toContain('DS_OK')
    expect(allNarration(steps)).toContain('STR_MAX_SIZE=100')
    expect(allNarration(steps)).toContain('报错') // 报错不截断口径
    expectStringContract(steps)
  })
  it('纯函数', () => { expectPure(stringStaticAssignSteps, input) })
})

// ---------- 2. heap-string-grow ----------
describe('heapStringGrow 生成器', () => {
  const input = { src: 'abcdefgh' }
  it('容量满 → 双倍新块 → 逐字符搬运 → 旧行 strike 标释放', () => {
    const steps = heapStringGrowSteps(input)
    const o = last(steps).state as any
    expect(o.rows).toHaveLength(2)
    expect(o.rows[0].strike).toEqual([0, 1, 2, 3])
    expect(o.rows[1].chars).toEqual(['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', '\\0'])
    expect(o.rows[1].capacity).toBe(9)   // 容量 8 + 1 格给 '\0'（realloc(need+1) 口径）
    const all = allNarration(steps)
    expect(all).toContain('2×4 = 8')
    expect(all).toContain('realloc')
    expect(all).toContain('DS_ERROR')
    expect(all).toContain('free(旧块)')
    expectStringContract(steps)
  })
  it('搬运帧 active 双行对应', () => {
    const steps = heapStringGrowSteps(input)
    const move = steps.filter(s => s.narration.includes('搬运'))
    expect(move).toHaveLength(8)
    expect((move[3].state as any).rows[0].charActive).toEqual([3])
    expect((move[3].state as any).rows[1].charActive).toEqual([3])
  })
  it('纯函数', () => { expectPure(heapStringGrowSteps, input) })
})

// ---------- 3. block-string-chunk ----------
describe('blockStringChunk 生成器', () => {
  const input = { src: 'ababcabcasd' } // 11 字符 → 3 块，尾块 3 有效 + 1 个 '#'
  it('CHUNK_SIZE=4 分块：终帧 3 行、尾块 # 填充、密度写入 note', () => {
    const steps = blockStringChunkSteps(input)
    const o = last(steps).state as any
    expect(o.rows).toHaveLength(3)
    expect(o.rows[2].chars).toEqual(['a', 's', 'd', '#'])
    expect(o.rows[0].chars).toEqual(['a', 'b', 'a', 'b'])
    const all = allNarration(steps)
    expect(all).toContain('CHUNK_SIZE = 4')
    expect(all).toContain('第 2 块第 2 槽')        // 逻辑下标换算代入（i=10 → 块 2 槽 2）
    expect((last(steps).state as any).note).toContain('存储密度 11/12')
    expect(all.match(/开新块/g)?.length).toBe(3)   // 第 0/1/2 块各开一次
    expectStringContract(steps)
  })
  it('满 12 字符时尾块恰好装满（无 # 填充）', () => {
    const steps = blockStringChunkSteps({ src: 'abcdefghijkl' })
    const o = last(steps).state as any
    expect(o.rows[2].chars).toEqual(['i', 'j', 'k', 'l'])
    expectStringContract(steps)
  })
  it('纯函数', () => { expectPure(blockStringChunkSteps, input) })
})

// ---------- 4. bf-match ----------
describe('bfMatch 生成器', () => {
  const input = { s: 'ababcabcacbab', t: 'abcac' }
  it('教材回退口径 + 命中：i-j+2 出现在失配帧、终帧 pos=6、比较 16 次', () => {
    const steps = bfMatchSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('i 回到 i-j+2')
    expect(all).toContain('start: 2 → 3')
    expect(last(steps).narration).toContain('匹配成功')
    expect(last(steps).narration).toContain('*pos = start+1 = 6')
    expect(last(steps).narration).toContain('比较 16 次')
    const o = last(steps).state as any
    expect(o.rows[0].charHighlight).toEqual([5, 6, 7, 8, 9])   // 主串命中段（0-based）
    expect(o.rows[1].charHighlight).toEqual([0, 1, 2, 3, 4])
    expectStringContract(steps)
  })
  it('失配回退段标灰色删除线', () => {
    const steps = bfMatchSteps(input)
    const miss = steps.find(s => s.narration.includes('✗ 失配'))
    expect(miss).toBeTruthy()
    expect((miss!.state as any).rows[0].strike.length).toBeGreaterThan(0)
    expect((miss!.state as any).rows[1].pointers[0].index).toBe(0)  // j 归 0（0-based）
  })
  it('未命中：DS_ERROR 收尾', () => {
    const steps = bfMatchSteps({ s: 'abab', t: 'abc' })
    expect(last(steps).narration).toContain('未找到匹配位置')
    expect(last(steps).narration).toContain('DS_ERROR')
    expectStringContract(steps)
  })
  it('纯函数', () => { expectPure(bfMatchSteps, input) })
})

// ---------- 5. kmp-match ----------
describe('kmpMatch 生成器', () => {
  const input = { s: 'ababcabcacbab', t: 'abcac' }
  it('next 表先算后用：next[1..5] = [0,1,1,1,2]（1-based 语义）', () => {
    const steps = kmpMatchSteps(input)
    const nextFrames = steps.filter(s => /^next\[\d+\] =/.test(s.narration))
    expect(nextFrames.map(s => (s.narration.match(/^next\[(\d+)\] = (\d+)/) ?? []).slice(1).join(':')))
      .toEqual(['2:1', '3:1', '4:1', '5:2'])
    const done = steps.find(s => s.narration.includes('next 表就绪'))
    expect((done!.state as any).rows[2].chars).toEqual([0, 1, 1, 1, 2])
    expect(done!.narration).toContain('next[1]=0、next[2]=1')
  })
  it('匹配：i 不回退、j 跳 next[j+1]（narration 给 next 值）、命中 pos=6、比较 12 次少于 BF', () => {
    const steps = kmpMatchSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('i 不回退！查 next[5] = 2，j 跳到 1')
    expect(all).toContain('失配于模式第 5 位')
    expect(last(steps).narration).toContain('匹配成功')
    expect(last(steps).narration).toContain('*pos = i-m+1 = 6')
    expect(last(steps).narration).toContain('共比较 12 次')
    expect(last(steps).narration).toContain('没回退过一步')
    const o = last(steps).state as any
    expect(o.rows[0].charHighlight).toEqual([5, 6, 7, 8, 9])
    expectStringContract(steps)
  })
  it('KMP 失配帧不含删除线（对照 BF：i 从不回退），未命中给 DS_ERROR', () => {
    const steps = kmpMatchSteps(input)
    for (const s of steps) {
      const strikeLen = ((s.state as any).rows[0].strike ?? []).length
      expect(strikeLen, 'KMP 主串不应有回退删除线').toBe(0)
    }
    const miss = kmpMatchSteps({ s: 'abab', t: 'abc' })
    expect(last(miss).narration).toContain('DS_ERROR')
    expectStringContract(miss)
  })
  it('纯函数', () => { expectPure(kmpMatchSteps, input) })
})

// ---------- 6. adjlist-build ----------
describe('adjlistBuild 生成器', () => {
  const input = { edges: [{ u: 0, v: 1 }, { u: 1, v: 2 }, { u: 2, v: 3 }, { u: 0, v: 3 }], n: 4 }
  it('每边对称挂两个弧结点：终帧 8 结点、aux 链序为尾插插入序', () => {
    const steps = adjlistBuildSteps(input)
    const o = last(steps).state as any
    expect(o.aux.map((a: any) => a.text)).toEqual([
      'first[0] → (1)→(3) → NULL',
      'first[1] → (0)→(2) → NULL',
      'first[2] → (1)→(3) → NULL',
      'first[3] → (2)→(0) → NULL'
    ])
    const all = allNarration(steps)
    expect(all).toContain('尾插')
    expect(all).toContain('头插')
    expect(all).toContain('8 个弧结点')
    expect(all).toContain('8 = 2×4')
    expect(last(steps).narration).toContain('O(n+e)')
    expectGraphContract(steps)
  })
  it('环状布局坐标在 0~100、结点数 = n', () => {
    const steps = adjlistBuildSteps(input)
    expect((steps[0].state as any).nodes).toHaveLength(4)
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(adjlistBuildSteps, input) })
})

// ---------- 7. orthlist-build ----------
describe('orthlistBuild 生成器', () => {
  const input = { edges: [{ u: 0, v: 1 }, { u: 1, v: 2 }, { u: 2, v: 0 }, { u: 1, v: 3 }], n: 4 }
  it('一弧一结点双链头插：出弧链逆插入序、入弧链同结点', () => {
    const steps = orthlistBuildSteps(input)
    const o = last(steps).state as any
    expect(o.aux[1].text).toContain('出弧链 first_out → (→v3)→(→v2)')  // v1 出弧：3 新、2 旧
    expect(o.aux[0].text).toContain('入弧链 first_in → (→v2)')          // v0 入弧：只有 2→0
    expect(o.aux[3].text).toContain('入弧链 first_in → (→v1)')
    const all = allNarration(steps)
    expect(all).toContain('tailvex=1、headvex=3')
    expect(all).toContain('一次 malloc 两根针')
    expect(all).toContain('头插')
    expect(last(steps).narration).toContain('4 个弧结点')
    expect(o.edges.every((e: any) => e.directed)).toBe(true)
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(orthlistBuildSteps, input) })
})

// ---------- 8. amledg-build ----------
describe('amlBuild 生成器', () => {
  const input = { edges: [{ u: 0, v: 1 }, { u: 1, v: 2 }, { u: 2, v: 3 }, { u: 0, v: 2 }], n: 4 }
  it('每边一结点两端串接：与邻接表 2e 对照、看路牌走链', () => {
    const steps = amlBuildSteps(input)
    const o = last(steps).state as any
    expect(o.aux[0].text).toContain('e(0,2)—e(0,1)')
    expect(o.aux[2].text).toContain('e(0,2)—e(2,3)—e(1,2)')
    const all = allNarration(steps)
    expect(all).toContain('边结点数 4 而非邻接表的 8')
    expect(last(steps).narration).toContain('4 个边结点（邻接表要 2e = 8 个）')
    expect(last(steps).narration).toContain('看路牌走链')
    expect(last(steps).narration).toContain('ilink : jlink')
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(amlBuildSteps, input) })
})

// ---------- 9. mst-prim ----------
describe('mstPrim 生成器', () => {
  const input = { edges: [{ u: 0, v: 1, w: 4 }, { u: 0, v: 2, w: 1 }, { u: 2, v: 1, w: 2 }, { u: 1, v: 3, w: 5 }, { u: 2, v: 3, w: 8 }], n: 4 }
  it('从 0 号点起：选边 0-2(1)、2-1(2)、1-3(5)，总权 8', () => {
    const steps = mstPrimSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('最小者 0-2（权 1）')
    expect(all).toContain('松弛：lowcost[1] = min(4, w[2][1] = 2) = 2')
    expect(all).toContain('lowcost[3] = min(8, w[1][3] = 5) = 5')
    expect(last(steps).narration).toContain('总权 = 8')
    const o = last(steps).state as any
    expect(o.edges.filter((e: any) => e.highlight)).toHaveLength(3)
    expect(o.nodes.every((nd: any) => nd.highlight)).toBe(true)
    expectGraphContract(steps)
  })
  it('不连通给出失败帧（源码 DS_ERROR 口径）', () => {
    const steps = mstPrimSteps({ edges: [{ u: 0, v: 1, w: 2 }], n: 4 })
    expect(last(steps).narration).toContain('图不连通')
    expect(last(steps).narration).toContain('DS_ERROR')
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(mstPrimSteps, input) })
})

// ---------- 10. mst-kruskal ----------
describe('mstKruskal 生成器', () => {
  const input = { edges: [{ u: 0, v: 1, w: 1 }, { u: 1, v: 2, w: 2 }, { u: 0, v: 2, w: 3 }, { u: 1, v: 3, w: 4 }, { u: 2, v: 3, w: 5 }], n: 4 }
  it('按权排序逐条尝试：0-2 成环舍弃、凑满 n-1 即停、总权 7', () => {
    const steps = mstKruskalSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('按权升序排好')
    expect(all).toContain('加这条边必成环，舍弃')
    expect(all).toContain('尝试 0-2（权 3）：find 两端同根')
    expect(all).toContain('parent[2] = 0')   // 按大小合并方向
    expect(last(steps).narration).toContain('n-1 = 3 条边即停')
    expect(last(steps).narration).toContain('总权 = 7')
    const o = last(steps).state as any
    expect(o.edges.filter((e: any) => e.highlight)).toHaveLength(3)
    expectGraphContract(steps)
  })
  it('并查集 parent 数组随合并演化（aux 可查）', () => {
    const steps = mstKruskalSteps(input)
    const acceptFrames = steps.filter(s => s.narration.includes('边入选'))
    expect(acceptFrames).toHaveLength(3)
    expect((last(acceptFrames).state as any).aux[0].text).toContain('parent: [0, 0, 0, 0]')
  })
  it('不连通给出失败帧', () => {
    const steps = mstKruskalSteps({ edges: [{ u: 0, v: 1, w: 2 }], n: 4 })
    expect(last(steps).narration).toContain('图不连通')
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(mstKruskalSteps, input) })
})

// ---------- 11. sp-dijkstra ----------
describe('spDijkstra 生成器', () => {
  const input = {
    edges: [{ u: 0, v: 1, w: 4 }, { u: 0, v: 2, w: 1 }, { u: 2, v: 1, w: 2 }, { u: 1, v: 3, w: 5 }, { u: 2, v: 3, w: 8 }],
    n: 4, src: 0
  }
  it('每轮定入 dist 最小者并松弛：dist 终值 [0,3,1,8]，prev 记录中转', () => {
    const steps = spDijkstraSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('有向网')
    expect(all).toContain('松弛弧 0→1（权 4）：dist[1] = ∞ > dist[0] + w = 0 + 4 = 4 → 更新为 4')
    expect(all).toContain('松弛弧 2→1（权 2）：dist[1] = 4 > dist[2] + w = 1 + 2 = 3 → 更新为 3，prev[1] = 2')
    expect(last(steps).narration).toContain('dist = [0, 3, 1, 8]')
    const o = last(steps).state as any
    expect(o.aux[0].text).toContain('dist:  [0, 3, 1, 8]')
    expect(o.aux[0].text).toContain('prev:  [−, 2, 0, 1]')
    expect(o.nodes.every((nd: any) => nd.highlight)).toBe(true)
    expect(o.edges.every((e: any) => e.directed)).toBe(true)
    expectGraphContract(steps)
  })
  it('不可达点提前 break 给说明帧', () => {
    const steps = spDijkstraSteps({ edges: [{ u: 0, v: 1, w: 2 }], n: 4, src: 0 })
    expect(steps.some(s => s.narration.includes('提前 break'))).toBe(true)
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(spDijkstraSteps, input) })
})

// ---------- 12. topo-kahn ----------
describe('topoKahn 生成器', () => {
  const input = { edges: [{ u: 0, v: 1 }, { u: 0, v: 2 }, { u: 2, v: 1 }, { u: 1, v: 3 }, { u: 2, v: 3 }], n: 4 }
  it('入度 0 入队 → 出队 → 削后继：拓扑序 0→2→1→3，narration 给序号与入度变化', () => {
    const steps = topoKahnSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('出队输出 v0（第 1 个）')
    expect(all).toContain('indegree[2] = 1 → 0，归零 → 入队')
    expect(all).toContain('indegree[3] = 2 → 1')
    expect(last(steps).narration).toContain('v0 → v2 → v1 → v3')
    expect(last(steps).narration).toContain('共 4 = n 个全输出，无环')
    const o = last(steps).state as any
    expect(o.aux[0].text).toContain('已输出: v0 → v2 → v1 → v3')
    expect(o.nodes.every((nd: any) => nd.highlight)).toBe(true)
    expectGraphContract(steps)
  })
  it('环输入给失败帧（队列空仍有剩余点）', () => {
    const cyc = topoKahnSteps({ edges: [{ u: 0, v: 1 }, { u: 1, v: 0 }], n: 2 })
    expect(last(cyc).narration).toContain('构成有向环')
    expect(last(cyc).narration).toContain('DS_ERROR')
    expectGraphContract(cyc)
  })
  it('纯函数', () => { expectPure(topoKahnSteps, input) })
})

// ---------- 13. external-merge ----------
describe('externalMerge 生成器', () => {
  const input = { segs: [[1, 3, 5], [2, 4, 6], [0, 7]] }
  it('逐轮选段首最小写入输出：终帧输出 [0..7]、narration 给来源段', () => {
    const steps = externalMergeSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('比较各段队首：1(f0), 2(f1), 0(f2) → 全局最小来自段 f2（值 0）')
    expect(all).toContain('写入 out：fprintf(out, "%d") = 0（来自段 f2）')
    expect(all).toContain('size = 4')
    expect(all).toContain('INT_MAX')
    expect(last(steps).narration).toContain('共写入 8 条')
    expect(last(steps).narration).toContain('[0, 1, 2, 3, 4, 5, 6, 7]')
    const o = last(steps).state as any
    expect(o.rows[3].chars).toEqual([0, 1, 2, 3, 4, 5, 6, 7])
    expect(o.rows).toHaveLength(4)              // 3 段 + 输出行
    expect(o.rows[0].strike).toEqual([0, 1, 2]) // f0 读尽整行删除线
    expectStringContract(steps)
  })
  it('每条输出配"比较 + 写入"两帧，EOF 段出说明帧', () => {
    const steps = externalMergeSteps(input)
    expect(steps.filter(s => s.narration.includes('比较各段队首'))).toHaveLength(8)
    expect(steps.filter(s => s.narration.includes('读尽'))).toHaveLength(3)
    expectStringContract(steps)
  })
  it('纯函数', () => { expectPure(externalMergeSteps, input) })
})

// ---------- 14. cp-critical-path ----------
describe('cpCriticalPath 生成器', () => {
  const input = {
    edges: [{ u: 0, v: 1, w: 3 }, { u: 0, v: 2, w: 2 }, { u: 1, v: 3, w: 2 }, { u: 2, v: 3, w: 4 }, { u: 3, v: 4, w: 2 }, { u: 3, v: 5, w: 3 }, { u: 4, v: 5, w: 1 }],
    n: 6
  }
  it('ve 正推取 max、vl 逆推取 min：duration=9，关键活动染绿且不止一条路径', () => {
    const steps = cpCriticalPathSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('ve[3] = max(5, ve[2] + w(2,3) = 2 + 4 = 6) = 6')
    expect(all).toContain('vl[3] = min(9, vl[4] − w(3,4) = 8 − 2 = 6) = 6')
    expect(all).toContain('2→3: e=2, l=2 ★关键')
    expect(last(steps).narration).toContain('duration = ve[汇] = 9')
    expect(last(steps).narration).toContain('可能不唯一')
    const o = last(steps).state as any
    expect(o.edges.filter((e: any) => e.highlight).map((e: any) => `${e.from}-${e.to}`)).toEqual(['0-2', '2-3', '3-4', '3-5', '4-5'])
    expect(o.aux[0].text).toContain('ve: [0, 3, 2, 6, 8, 9]')
    expect(o.aux[0].text).toContain('vl: [0, 4, 2, 6, 8, 9]')
    expectGraphContract(steps)
  })
  it('多源多汇 / 环输入给失败帧', () => {
    const multi = cpCriticalPathSteps({ edges: [{ u: 0, v: 2, w: 1 }, { u: 1, v: 2, w: 2 }], n: 3 })
    expect(last(multi).narration).toContain('源点 2 个、汇点 1 个')
    // 单源单汇但内部有环：0→1、1→2、2→1、2→3
    const cyc = cpCriticalPathSteps({ edges: [{ u: 0, v: 1, w: 1 }, { u: 1, v: 2, w: 1 }, { u: 2, v: 1, w: 1 }, { u: 2, v: 3, w: 1 }], n: 4 })
    expect(last(cyc).narration).toContain('存在有向环')
    expectGraphContract(multi, 1)
    expectGraphContract(cyc, 1)
  })
  it('纯函数', () => { expectPure(cpCriticalPathSteps, input) })
})

// ---------- 15. crosslist-sparse ----------
describe('crosslistSparse 生成器', () => {
  const input = { cells: [{ i: 0, j: 1, v: 5 }, { i: 0, j: 3, v: 2 }, { i: 2, j: 1, v: 4 }], rows: 3, cols: 4 }
  it('行链列链双有序：终帧 aux 给 rhead/chead 全链、结点画在网格坐标', () => {
    const steps = crosslistSparseSteps(input)
    const o = last(steps).state as any
    expect(o.aux.map((a: any) => a.text)).toEqual([
      'rhead[0] → (0,1)=5─right→(0,3)=2 → NULL',
      'rhead[1] → NULL',
      'rhead[2] → (2,1)=4 → NULL',
      'chead[0] → NULL',
      'chead[1] → (0,1)=5─down→(2,1)=4 → NULL',
      'chead[2] → NULL',
      'chead[3] → (0,3)=2 → NULL'
    ])
    expect(o.nodes).toHaveLength(3)
    const all = allNarration(steps)
    expect(all).toContain('插到第 1 位')
    expect(all).toContain('缝隙')
    expect(last(steps).narration).toContain('right 横串列号增大、down 竖串行号增大')
    expectGraphContract(steps)
  })
  it('纯函数', () => { expectPure(crosslistSparseSteps, input) })
})

// ---------- registry：批4 新增键 ----------
const B4_STRING_KEYS = ['string-static-assign', 'heap-string-grow', 'block-string-chunk', 'bf-match', 'kmp-match', 'external-merge']
const B4_GRAPH_KEYS = ['adjlist-build', 'orthlist-build', 'amledg-build', 'mst-prim', 'mst-kruskal', 'sp-dijkstra', 'topo-kahn', 'cp-critical-path', 'crosslist-sparse']
const B4_KEYS = [...B4_STRING_KEYS, ...B4_GRAPH_KEYS]

describe('registry 批4 新增键', () => {
  it('15 个新键全部注册、renderer 为 string/graph、defaultInput 均通过自身校验', () => {
    for (const k of B4_STRING_KEYS) {
      const def = registry[k]
      expect(def, `缺键 ${k}`).toBeTruthy()
      expect(def.renderer, `${k} renderer`).toBe('string')
      expect(def.validate(def.parse(def.defaultInput))).toBeNull()
    }
    for (const k of B4_GRAPH_KEYS) {
      const def = registry[k]
      expect(def, `缺键 ${k}`).toBeTruthy()
      expect(def.renderer, `${k} renderer`).toBe('graph')
      expect(def.validate(def.parse(def.defaultInput))).toBeNull()
    }
  })
  it('全链路：defaultInput 经 parse→validate→steps 产出契约内有效帧（PlayerShell 同款调用序）', () => {
    for (const k of B4_KEYS) {
      const def = registry[k]
      const steps = def.steps(def.parse(def.defaultInput))
      expect(steps.length, `${k} 帧数不足`).toBeGreaterThan(1)
      for (const s of steps) expect(s.narration.length, `${k} 空 narration`).toBeGreaterThan(0)
      ;(def.renderer === 'string' ? expectStringContract : expectGraphContract)(steps)
    }
  })
  it('非法输入返回中文提示（含示例字样"如"）', () => {
    const R = registry
    expect(R['string-static-assign'].validate(R['string-static-assign'].parse('123456789012345'))).toContain('1~12 个字符')
    expect(R['heap-string-grow'].validate(R['heap-string-grow'].parse('abc'))).toContain('5~8')
    expect(R['bf-match'].validate(R['bf-match'].parse('abcabc'))).toContain('主串|模式串')
    expect(R['bf-match'].validate(R['bf-match'].parse('ab|abc'))).toContain('不能长于主串')
    expect(R['kmp-match'].validate(R['kmp-match'].parse('a b|c'))).toContain('不能含空白')
    expect(R['adjlist-build'].validate(R['adjlist-build'].parse('0-1,0-1|4'))).toContain('重复边')
    expect(R['orthlist-build'].validate(R['orthlist-build'].parse('0-9|4'))).toContain('0~3')
    expect(R['mst-prim'].validate(R['mst-prim'].parse('0-1,1-2|4'))).toContain('带权')
    expect(R['mst-kruskal'].validate(R['mst-kruskal'].parse('0-1:0,1-2:2|4'))).toContain('权值需为 1~99')
    expect(R['sp-dijkstra'].validate(R['sp-dijkstra'].parse('0-1:4|4|9'))).toContain('源点需为 0~3')
    expect(R['topo-kahn'].validate(R['topo-kahn'].parse('0->1,1->0|2'))).toContain('存在环')
    expect(R['cp-critical-path'].validate(R['cp-critical-path'].parse('0->1:1,1->0:1|2'))).toContain('不允许成环')
    expect(R['crosslist-sparse'].validate(R['crosslist-sparse'].parse('0-1:5,0-1:2|3|4'))).toContain('重复设置')
    expect(R['external-merge'].validate(R['external-merge'].parse('5,3,1|2,4'))).toContain('升序')
    expect(R['block-string-chunk'].validate(R['block-string-chunk'].parse('a b'))).toContain('空白')
  })
})
