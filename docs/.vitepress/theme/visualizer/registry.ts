// docs/.vitepress/theme/visualizer/registry.ts
import type { VisualizerDef } from './types'
import { insertionSortSteps } from './steps/insertionSort'
import { shellSortSteps } from './steps/shellSort'
import { bubbleSortSteps } from './steps/bubbleSort'
import { quickSortSteps } from './steps/quickSort'
import { selectionSortSteps } from './steps/selectionSort'
import { heapSortSteps } from './steps/heapSort'
import { mergeSortSteps } from './steps/mergeSort'
import { countingSortSteps } from './steps/countingSort'
import { radixSortSteps } from './steps/radixSort'
// 批1：线性表 / 矩阵压缩 / 图建图 / 查找 / 散列
import { seqListInsertSteps } from './steps/seqListInsert'
import { seqListRemoveSteps } from './steps/seqListRemove'
import { dynaSeqGrowSteps } from './steps/dynaSeqGrow'
import { staticListCursorSteps } from './steps/staticListCursor'
import { symmetricCompressSteps } from './steps/symmetricCompress'
import { triangularCompressSteps } from './steps/triangularCompress'
import { diagonalCompressSteps } from './steps/diagonalCompress'
import { tripleTransposeSteps } from './steps/tripleTranspose'
import { seqSearchSteps } from './steps/seqSearch'
import { binSearchSteps } from './steps/binSearch'
import { blockSearchSteps } from './steps/blockSearch'
import { hashFuncMapSteps } from './steps/hashFuncMap'
import { hashOpenProbeSteps } from './steps/hashOpenProbe'
import { hashAslCompareSteps } from './steps/hashAslCompare'
import { graphMatrixBuildSteps } from './steps/graphMatrixBuild'
import { hashChainInsertSteps } from './steps/hashChainInsert'
// 批2：栈 / 队列 / 链表（StackQueue / ListNode 双渲染器）
import { singlyLinkedInsertSteps } from './steps/singlyLinkedInsert'
import { doublyLinkedInsertSteps } from './steps/doublyLinkedInsert'
import { josephusRingSteps } from './steps/josephusRing'
import { seqStackSteps } from './steps/seqStack'
import { sharedStackSteps } from './steps/sharedStack'
import { linkStackSteps } from './steps/linkStack'
import { bracketMatchSteps } from './steps/bracketMatch'
import { circularQueueSteps } from './steps/circularQueue'
import { linkQueueSteps } from './steps/linkQueue'
import { dancePartnerSteps } from './steps/dancePartner'
// 批3：树类（Tree 渲染器）
import { treeStorageViewsSteps } from './steps/treeStorageViews'
import { seqBitreeIndexSteps } from './steps/seqBitreeIndex'
import { bitreeTraverseSteps, type TraverseMode } from './steps/bitreeTraverse'
import { threadBuildSteps } from './steps/threadBuild'
import { forestConvertSteps } from './steps/forestConvert'
import { huffmanBuildSteps } from './steps/huffmanBuild'
import { unionFindOpsSteps } from './steps/unionFindOps'
import { bstInsertSearchSteps } from './steps/bstInsertSearch'
import { avlRotateSteps, avlRotationCount } from './steps/avlRotate'
import { rbInsertSteps } from './steps/rbInsert'
import { btreeSplitSteps } from './steps/btreeSplit'
import { bplusInsertSteps } from './steps/bplusInsert'
import { preNullBuild } from './steps/treeKit'
// 批4：串 / 图存储与算法 / 外排（String / Graph 双渲染器）
import { stringStaticAssignSteps } from './steps/stringStaticAssign'
import { heapStringGrowSteps } from './steps/heapStringGrow'
import { blockStringChunkSteps } from './steps/blockStringChunk'
import { bfMatchSteps } from './steps/bfMatch'
import { kmpMatchSteps } from './steps/kmpMatch'
import { adjlistBuildSteps } from './steps/adjlistBuild'
import { orthlistBuildSteps } from './steps/orthlistBuild'
import { amlBuildSteps } from './steps/amlBuild'
import { mstPrimSteps } from './steps/mstPrim'
import { mstKruskalSteps } from './steps/mstKruskal'
import { spDijkstraSteps } from './steps/spDijkstra'
import { topoKahnSteps } from './steps/topoKahn'
import { externalMergeSteps } from './steps/externalMerge'
import { cpCriticalPathSteps } from './steps/cpCriticalPath'
import { crosslistSparseSteps } from './steps/crosslistSparse'

function numberList(min: number, max: number, maxLen: number) {
  return {
    parse: (text: string) => text.split(/[,，\s]+/).filter(s => s.length).map(Number),
    validate: (input: unknown): string | null => {
      if (!Array.isArray(input) || input.length === 0) return '至少输入 1 个数字'
      if (input.length > maxLen) return `最多 ${maxLen} 个元素`
      if (input.some(v => !Number.isInteger(v) || v < min || v > max)) return `值需为 ${min}~${max} 的整数`
      return null
    }
  }
}

const sortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 999, 20)
})
const bucketSortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 99, 20)
})
const countingDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 9, 20)
})

// ---------- 批1 通用 parse/validate 小工具 ----------
/** 逗号/空白分隔的整数段；出现非整数返回 null */
function numSeg(s: string): number[] | null {
  const arr = s.split(/[,，\s]+/).filter(Boolean).map(Number)
  if (arr.some(v => !Number.isInteger(v))) return null
  return arr
}
/** 按 | 切分输入段（全部非空），段数不符返回 null */
function segs(text: string, count: number): string[] | null {
  const parts = text.split('|').map(s => s.trim())
  return parts.length === count && parts.every(p => p.length) ? parts : null
}
const isPrime = (v: number) => {
  if (v < 2) return false
  for (let d = 2; d * d <= v; d++) if (v % d === 0) return false
  return true
}
const ERR = (msg: string, example: string) => `${msg}，如 ${example}`

// ---------- 批2 通用 parse/validate 小工具 ----------/** 栈类操作串："push:值" / "pop"（大小写兼容）；看不懂的片段原样带回待 validate 拒绝 */
function stackOpSeg(text: string) {
  return text.split(/[,，\s]+/).filter(Boolean).map(t => {
    const m = /^push:(-?\d+)$/i.exec(t)
    if (m) return { op: 'push', v: Number(m[1]) }
    if (/^pop$/i.test(t)) return { op: 'pop' as const }
    return { op: `看不懂的片段:${t}` }
  })
}
/** 队列操作串："en:值" / "de" */
function queueOpSeg(text: string) {
  return text.split(/[,，\s]+/).filter(Boolean).map(t => {
    const m = /^en:(-?\d+)$/i.exec(t)
    if (m) return { op: 'en', v: Number(m[1]) }
    if (/^de$/i.test(t)) return { op: 'de' as const }
    return { op: `看不懂的片段:${t}` }
  })
}
/** 校验操作串数组的通用检查（ops 形状 + push 值域） */
function checkOps(ops: { op: string; v?: number }[], maxOps: number, ex: string, enName: string): string | null {
  if (ops.length < 1) return ERR(`至少 1 个操作（${enName}）`, ex)
  if (ops.length > maxOps) return ERR(`最多 ${maxOps} 个操作`, ex)
  if (ops.some(o => o.op !== 'push' && o.op !== 'pop' && o.op !== 'en' && o.op !== 'de')) return ERR(`操作只认 ${enName}`, ex)
  if (ops.some(o => (o.op === 'push' || o.op === 'en') && (!Number.isInteger(o.v) || (o.v as number) < 1 || (o.v as number) > 999))) return ERR('入栈/入队值需为 1~999 的整数', ex)
  return null
}

// ---------- 批3 通用 parse/validate 小工具（树类） ----------
/** 唯一性检查：重复返回提示 */
function uniqErr(a: number[], name: string, ex: string): string | null {
  return new Set(a).size !== a.length ? ERR(`${name}不可重复（重复键会使树形语义失真）`, ex) : null
}
/** 带空标记先序串的键位检查：'#' 或单字符字母数字 */
function tokenErr(tokens: string[], ex: string): string | null {
  if (tokens.length < 1 || tokens.length > 25) return ERR('标记串需 1~25 个（逗号分隔）', ex)
  if (tokens.some(t => t !== '#' && !/^[A-Za-z0-9]$/.test(t))) return ERR('键位只认单个字母/数字，空子树用 # 表示', ex)
  return null
}
/** level 模式：不允许 # */
function levelTokenErr(tokens: string[], ex: string): string | null {
  if (tokens.some(t => t === '#')) return ERR('level 模式输入层序数据，不带空标记 #', ex)
  return null
}

// ---------- 批4 通用 parse/validate 小工具（串 / 图 / 外排） ----------
/** 边词元："u-v"、"u->v"、"u-v:w"、"u->v:w"（'->' 与 '-' 同义收下，方向语义由各动画说明） */
function edgeTok(s: string): { u: number; v: number; w?: number } | null {
  const m = /^(\d+)(?:->|-)(\d+)(?::(\d+))?$/.exec(s)
  return m ? { u: Number(m[1]), v: Number(m[2]), ...(m[3] !== undefined ? { w: Number(m[3]) } : {}) } : null
}
/** "边串|顶点数[|源点]" 解析；看不懂的词元变成 NaN 边交给 validate 拒绝 */
function parseEdges(text: string, segCount: 2 | 3) {
  const parts = segs(text, segCount)
  if (!parts) return { parts: null as string[] | null, edges: [] as { u: number; v: number; w?: number }[], n: NaN, src: NaN }
  const edges = parts[0].split(/[,，\s]+/).filter(Boolean).map(edgeTok).map(t => t ?? { u: NaN, v: NaN })
  return { parts, edges, n: Number(parts[1]), src: segCount === 3 ? Number(parts[2]) : NaN }
}
/** 无向边串通用校验（范围/自环/重复），directed 时按键 "u->v" 判重 */
function edgesErr(edges: { u: number; v: number }[], n: number, maxEdges: number, directed: boolean, ex: string): string | null {
  if (edges.length < 1 || edges.length > maxEdges) return ERR(`边需 1~${maxEdges} 条`, ex)
  if (edges.some(e => !Number.isInteger(e.u) || !Number.isInteger(e.v))) return ERR('边需为"u-v"（或"u->v"）两个整数', ex)
  if (edges.some(e => e.u < 0 || e.u >= n || e.v < 0 || e.v >= n || e.u === e.v)) return ERR(`顶点号需在 0~${n - 1} 且 u≠v`, ex)
  const seen = new Set<string>()
  for (const e of edges) {
    const key = directed ? `${e.u}->${e.v}` : ukeyOf(e.u, e.v)
    if (seen.has(key)) return ERR('存在重复边', ex)
    seen.add(key)
  }
  return null
}
const ukeyOf = (u: number, v: number) => (u <= v ? `${u}-${v}` : `${v}-${u}`)
/** 带权边校验：权值必须给出且为 min~99 的整数（"u-v:w"） */
function weightErr(edges: { w?: number }[], ex: string, min = 1): string | null {
  if (edges.some(e => e.w === undefined || !Number.isInteger(e.w))) return ERR('边需带权："u-v:w"（如 0-1:4）', ex)
  if (edges.some(e => (e.w as number) < min || (e.w as number) > 99)) return ERR(`权值需为 ${min}~99 的整数`, ex)
  return null
}
/** Kahn 判环：能完整出队 n 个则无环（拓扑排序 / AOE 前置校验共用） */
function hasCycle(edges: { u: number; v: number }[], n: number): boolean {
  const indeg = Array.from({ length: n }, () => 0)
  for (const e of edges) indeg[e.v]++
  const q: number[] = []
  for (let i = 0; i < n; i++) if (indeg[i] === 0) q.push(i)
  let seen = 0
  while (q.length) {
    const u = q.shift() as number
    seen++
    for (const e of edges) if (e.u === u && --indeg[e.v] === 0) q.push(e.v)
  }
  return seen < n
}
/** 源串校验：可见字符、无空白无 |（| 是分隔符）、限长 */
function srcStrErr(s: string, min: number, max: number, ex: string): string | null {
  if (!s || s.length < min || s.length > max) return ERR(min === 1 ? `源串需 1~${max} 个字符` : `源串需 ${min}~${max} 个字符${min > 1 ? `（≤${min - 1} 触发不了本动画的演示点）` : ''}`, ex)
  if (/[\s|]/.test(s)) return ERR('源串不能含空白与 |（| 是输入分隔符）', ex)
  return null
}

/** 段1=整数列表、段2=整数列表 的双段输入骨架（错误信息统一带示例）
 *  build: 把两段整数映射成 steps() 需要的输入形状 */
function pairListDef(
  title: string, steps: (x: any) => any, defaultInput: string,
  check: (left: number[], right: number[]) => string | null,
  build: (left: number[], right: number[]) => any
): VisualizerDef {
  return {
    title, renderer: 'arrayBar', defaultInput,
    parse: (text: string) => {
      const parts = segs(text, 2)
      const left = parts ? numSeg(parts[0]) : null
      const right = parts ? numSeg(parts[1]) : null
      return { parts, left, right, built: left && right ? build(left, right) : null }
    },
    validate: (input: any): string | null => {
      const ex = defaultInput
      if (!input.parts) return ERR('格式需用 | 分隔两段', ex)
      if (input.left === null || input.right === null || input.left.length === 0) return ERR('两段都需为整数列表', ex)
      return check(input.left, input.right)
    },
    // PlayerShell 只在 validate 通过后调用 steps，此时 built 必已就绪
    steps: (raw: any) => steps(raw.built)
  }
}

export const registry: Record<string, VisualizerDef> = {
  'insertion-sort': sortDef('插入排序', insertionSortSteps, '5,2,9,1,7'),
  'shell-sort': sortDef('希尔排序', shellSortSteps, '8,5,3,9,1,6'),
  'bubble-sort': sortDef('冒泡排序', bubbleSortSteps, '5,2,9,1,7'),
  'quick-sort': sortDef('快速排序', quickSortSteps, '5,2,9,1,7,3'),
  'selection-sort': sortDef('简单选择排序', selectionSortSteps, '5,2,9,1,7'),
  'heap-sort': sortDef('堆排序', heapSortSteps, '5,2,9,1,7,3'),
  'merge-sort': sortDef('归并排序', mergeSortSteps, '5,2,9,1,7,3'),
  'counting-sort': countingDef('计数排序（值域 0~9）', countingSortSteps, '3,1,4,1,5'),
  'radix-sort': bucketSortDef('基数排序（LSD，值域 0~99）', radixSortSteps, '42,7,91,7,30'),

  // ---------- 01_线性表 ----------
  'seqlist-insert': pairListDef('静态顺序表插入', seqListInsertSteps, '12,25,33,47,58|3,40', (list, pv) => {
    if (list.length < 1 || list.length > 15) return ERR('初始元素需 1~15 个', '12,25,33,47,58|3,40')
    if (list.some(v => v < 1 || v > 999)) return ERR('元素需为 1~999 的整数（0 是空槽标记）', '12,25,33,47,58|3,40')
    if (pv.length !== 2) return ERR('第二段为"位序,元素"两个整数', '12,25,33,47,58|3,40')
    const [pos, value] = pv
    if (pos < 1 || pos > list.length + 1) return ERR(`位序需在 1~${list.length + 1}（表尾追加即 ${list.length + 1}）`, '12,25,33,47,58|3,40')
    if (value < 1 || value > 999) return ERR('待插元素需为 1~999 的整数', '12,25,33,47,58|3,40')
    return null
  }, (list, pv) => ({ list, pos: pv[0], value: pv[1] })),
  'seqlist-remove': pairListDef('静态顺序表删除', seqListRemoveSteps, '12,25,33,47,58|2', (list, right) => {
    if (list.length < 1 || list.length > 15) return ERR('初始元素需 1~15 个', '12,25,33,47,58|2')
    if (list.some(v => v < 1 || v > 999)) return ERR('元素需为 1~999 的整数（0 是空槽标记）', '12,25,33,47,58|2')
    if (right.length !== 1) return ERR('第二段为待删位序一个整数', '12,25,33,47,58|2')
    const pos = right[0]
    if (pos < 1 || pos > list.length) return ERR(`位序需在 1~${list.length}`, '12,25,33,47,58|2')
    return null
  }, (list, right) => ({ list, pos: right[0] })),
  'dynaseq-grow': {
    title: '动态顺序表扩容搬家', renderer: 'arrayBar', steps: dynaSeqGrowSteps, defaultInput: '5,12,18,23',
    ...numberList(1, 999, 12)
  },
  'staticlist-cursor': pairListDef('静态链表游标插入', staticListCursorSteps, '10,20,30,40|3,25', (list, pv) => {
    if (list.length < 1 || list.length > 7) return ERR('数据分量需 1~7 个（下标 1~8，还要留位给新结点）', '10,20,30,40|3,25')
    if (list.some(v => v < 1 || v > 999)) return ERR('元素需为 1~999 的整数', '10,20,30,40|3,25')
    if (pv.length !== 2) return ERR('第二段为"位序,元素"两个整数', '10,20,30,40|3,25')
    const [pos, value] = pv
    if (pos < 1 || pos > list.length + 1) return ERR(`位序需在 1~${list.length + 1}`, '10,20,30,40|3,25')
    if (value < 1 || value > 999) return ERR('待插元素需为 1~999 的整数', '10,20,30,40|3,25')
    return null
  }, (list, pv) => ({ list, pos: pv[0], value: pv[1] })),

  // ---------- 04_特殊矩阵压缩存储 ----------
  'symmetric-compress': {
    title: '对称矩阵压缩映射', renderer: 'arrayBar', steps: symmetricCompressSteps, defaultInput: '1,2,4,3,5,7,6,8,9,10',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      if (!Array.isArray(a) || a.length === 0) return ERR('需为下三角元素逗号串', '1,2,4,3,5,7,6,8,9,10')
      if (![6, 10, 15].includes(a.length)) return ERR('下三角元素个数应为 6（3 阶）、10（4 阶）或 15（5 阶）', '1,2,4,3,5,7,6,8,9,10')
      if (a.some(v => v < 0 || v > 999)) return ERR('元素需为 0~999 的整数', '1,2,4,3,5,7,6,8,9,10')
      return null
    }
  },
  'triangular-compress': pairListDef('三角矩阵压缩（下三角+常数 c）', triangularCompressSteps, '1,2,4,3,5,7|9', (values, right) => {
    if (![6, 10, 15].includes(values.length)) return ERR('下三角元素个数应为 6（3 阶）、10（4 阶）或 15（5 阶）', '1,2,4,3,5,7|9')
    if (values.some(v => v < 0 || v > 999)) return ERR('元素需为 0~999 的整数', '1,2,4,3,5,7|9')
    if (right.length !== 1 || right[0] < 0 || right[0] > 999) return ERR('第二段为常数 c（0~999 整数）', '1,2,4,3,5,7|9')
    return null
  }, (values, right) => ({ values, c: right[0] })),
  'diagonal-compress': {
    title: '三对角矩阵压缩（k=2i+j−3）', renderer: 'arrayBar', steps: diagonalCompressSteps, defaultInput: '4,7,3,9,5,8,6',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      if (!Array.isArray(a) || a.length === 0) return ERR('需为三对角元素逗号串（按行序）', '4,7,3,9,5,8,6')
      if (![4, 7, 10, 13].includes(a.length)) return ERR('带内元素个数应为 4（2 阶）、7（3 阶）、10（4 阶）或 13（5 阶）', '4,7,3,9,5,8,6')
      if (a.some(v => v < 1 || v > 999)) return ERR('元素需为 1~999 的整数（带外才是 0）', '4,7,3,9,5,8,6')
      return null
    }
  },
  'triple-transpose': {
    title: '稀疏矩阵快速转置（cpot 定位）', renderer: 'arrayBar', steps: tripleTransposeSteps, defaultInput: '1-2-12,2-2-9,3-1-5,3-4-7|4',
    parse: (text: string) => {
      const parts = segs(text, 2)
      if (!parts) return { parts: null, triples: [], cols: NaN }
      const triples = parts[0].split(/[,，\s]+/).filter(Boolean).map(s => {
        const [r, c, v] = s.split('-').map(Number)
        return { r, c, v }
      })
      const cols = Number(parts[1])
      return { parts, triples, cols }
    },
    validate: (input: any): string | null => {
      const ex = '1-2-12,2-2-9,3-1-5,3-4-7|4'
      if (!input.parts) return ERR('格式："行-列-值"串|列数', ex)
      const { triples, cols } = input
      if (!Number.isInteger(cols) || cols < 1 || cols > 6) return ERR('列数需为 1~6 的整数', ex)
      if (triples.length < 1 || triples.length > 10) return ERR('三元组需 1~10 个', ex)
      if (triples.some((t: any) => !Number.isInteger(t.r) || !Number.isInteger(t.c) || !Number.isInteger(t.v))) return ERR('三元组需为"行-列-值"三个整数', ex)
      if (triples.some((t: any) => t.r < 1 || t.r > 6 || t.c < 1 || t.c > cols)) return ERR(`行号 1~6、列号 1~${cols}`, ex)
      if (triples.some((t: any) => t.v < 1 || t.v > 999)) return ERR('值需为 1~999 的非零整数', ex)
      for (let i = 1; i < triples.length; i++) {
        const p = triples[i - 1], q = triples[i]
        if (q.r < p.r || (q.r === p.r && q.c <= p.c)) return ERR('三元组需按行序（先按行、行内按列）排列', ex)
      }
      return null
    }
  },

  // ---------- 07_图 ----------
  'graph-matrix-build': {
    title: '邻接矩阵建图', renderer: 'arrayBar', steps: graphMatrixBuildSteps, defaultInput: '0-1,1-2,2-3|4',
    parse: (text: string) => {
      const parts = segs(text, 2)
      if (!parts) return { parts: null, edges: [], n: NaN, directed: false }
      const mode = parts[1].split(/[,，\s]+/).filter(Boolean)
      const edges = parts[0].split(/[,，\s]+/).filter(Boolean).map(s => {
        const [u, v] = s.split('-').map(Number)
        return { u, v }
      })
      return { parts, edges, n: Number(mode[0]), directed: mode[1] === '有向' }
    },
    validate: (input: any): string | null => {
      const ex = '0-1,1-2,2-3|4'
      if (!input.parts) return ERR('格式："u-v"边串|顶点数（可加 ",有向"）', ex)
      const { edges, n, directed } = input
      if (!Number.isInteger(n) || n < 2 || n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      if (edges.length < 1 || edges.length > 12) return ERR('边需 1~12 条', ex)
      if (edges.some((e: any) => !Number.isInteger(e.u) || !Number.isInteger(e.v))) return ERR('边需为"u-v"两个整数', ex)
      if (edges.some((e: any) => e.u < 0 || e.u >= n || e.v < 0 || e.v >= n || e.u === e.v)) return ERR(`顶点号需在 0~${n - 1} 且 u≠v`, ex)
      const seen = new Set<string>()
      for (const e of edges) {
        const key = directed ? `${e.u}-${e.v}` : [e.u, e.v].sort((a, b) => a - b).join('-')
        if (seen.has(key)) return ERR('存在重复边', ex)
        seen.add(key)
      }
      return null
    }
  },

  // ---------- 08_查找 ----------
  'seq-search': pairListDef('顺序查找（哨兵版）', seqSearchSteps, '3,7,1,9,5|9', (a, right) => {
    if (a.length < 1 || a.length > 15) return ERR('查找表需 1~15 个元素', '3,7,1,9,5|9')
    if (a.some(v => v < 0 || v > 999)) return ERR('元素需为 0~999 的整数', '3,7,1,9,5|9')
    if (right.length !== 1 || right[0] < 0 || right[0] > 999) return ERR('第二段为待查目标（0~999 整数）', '3,7,1,9,5|9')
    return null
  }, (a, right) => ({ a, key: right[0] })),
  'bin-search': pairListDef('折半查找', binSearchSteps, '1,3,5,7,9,11|7', (a, right) => {
    if (a.length < 1 || a.length > 16) return ERR('查找表需 1~16 个元素', '1,3,5,7,9,11|7')
    if (a.some(v => v < 0 || v > 999)) return ERR('元素需为 0~999 的整数', '1,3,5,7,9,11|7')
    for (let i = 1; i < a.length; i++) {
      if (a[i] < a[i - 1]) return ERR('查找表需按升序排列（折半的前提）', '1,3,5,7,9,11|7')
    }
    if (right.length !== 1 || right[0] < 0 || right[0] > 999) return ERR('第二段为待查目标（0~999 整数）', '1,3,5,7,9,11|7')
    return null
  }, (a, right) => ({ a, key: right[0] })),
  'block-search': {
    title: '分块查找', renderer: 'arrayBar', steps: blockSearchSteps, defaultInput: '5,8,3,12,15,22,18,25,30,27,42,36|4|25',
    parse: (text: string) => {
      const parts = segs(text, 3)
      return { parts, a: parts ? numSeg(parts[0]) : null, blockSize: parts ? Number(parts[1]) : NaN, key: parts ? Number(parts[2]) : NaN }
    },
    validate: (input: any): string | null => {
      const ex = '5,8,3,12,15,22,18,25,30,27,42,36|4|25'
      if (!input.parts) return ERR('格式：数据串|块大小|目标', ex)
      const { a, blockSize, key } = input
      if (!a || a.length < 4 || a.length > 18) return ERR('全表需 4~18 个元素', ex)
      if (a.some((v: number) => v < 0 || v > 999)) return ERR('元素需为 0~999 的整数', ex)
      if (!Number.isInteger(blockSize) || blockSize < 2 || blockSize > 6) return ERR('块大小需为 2~6 的整数', ex)
      if (blockSize >= a.length) return ERR('块大小需小于全表长度，否则不成块', ex)
      for (let i = 0; i + blockSize < a.length; i += blockSize) {
        const curMax = Math.max(...a.slice(i, i + blockSize))
        const nextMin = Math.min(...a.slice(i + blockSize, i + 2 * blockSize))
        if (curMax >= nextMin) return ERR('需块间有序：前一块所有元素应小于后一块（max 更小），块内可乱序', ex)
      }
      if (!Number.isInteger(key) || key < 0 || key > 999) return ERR('目标需为 0~999 的整数', ex)
      return null
    }
  },
  'hash-func-map': pairListDef('散列函数：除留余数映射', hashFuncMapSteps, '12,39,18,24,33,21|7', (keys, right) => {
    if (keys.length < 1 || keys.length > 15) return ERR('关键字需 1~15 个', '12,39,18,24,33,21|7')
    if (keys.some(v => v < 0 || v > 999)) return ERR('关键字需为 0~999 的整数', '12,39,18,24,33,21|7')
    if (right.length !== 1 || !Number.isInteger(right[0]) || right[0] < 2 || right[0] > 13 || !isPrime(right[0])) return ERR('模 p 需为 2~13 的素数', '12,39,18,24,33,21|7')
    return null
  }, (keys, right) => ({ keys, p: right[0] })),
  'hash-chain-insert': pairListDef('拉链法插入', hashChainInsertSteps, '12,39,18,24,33,21|7', (keys, right) => {
    if (keys.length < 1 || keys.length > 12) return ERR('关键字需 1~12 个', '12,39,18,24,33,21|7')
    if (new Set(keys).size !== keys.length) return ERR('关键字不可重复（模块对重复键返回"关键字已存在"）', '12,39,18,24,33,21|7')
    if (keys.some(v => v < 0 || v > 999)) return ERR('关键字需为 0~999 的整数', '12,39,18,24,33,21|7')
    if (right.length !== 1 || !Number.isInteger(right[0]) || right[0] < 3 || right[0] > 13 || !isPrime(right[0])) return ERR('桶数 p 需为 3~13 的素数', '12,39,18,24,33,21|7')
    return null
  }, (keys, right) => ({ keys, p: right[0] })),
  'hash-open-probe': pairListDef('开放定址·二次探测插入', hashOpenProbeSteps, '18,24,39,60,45|7', (keys, right) => {
    const ex = '18,24,39,60,45|7'
    if (right.length !== 1 || !Number.isInteger(right[0]) || right[0] < 3 || right[0] > 13 || !isPrime(right[0])) return ERR('表长 p 需为 3~13 的素数（建议 4k+3 型）', ex)
    const p = right[0]
    if (keys.length < 1 || keys.length > p - 1) return ERR(`关键字需 1~${p - 1} 个（至少留一空格，α<1）`, ex)
    if (new Set(keys).size !== keys.length) return ERR('关键字不可重复（模块对重复键返回"关键字已存在"）', ex)
    if (keys.some(v => v < 0 || v > 999)) return ERR('关键字需为 0~999 的整数', ex)
    return null
  }, (keys, right) => ({ keys, p: right[0] })),
  'hash-asl-compare': {
    title: '散列 ASL 成功/失败对比', renderer: 'arrayBar', steps: hashAslCompareSteps, defaultInput: '12,39,18,24,33,21',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '12,39,18,24,33,21'
      if (!Array.isArray(a) || a.length < 2 || a.length > 12) return ERR('关键字需 2~12 个', ex)
      if (new Set(a).size !== a.length) return ERR('关键字不可重复（链上同键会让 ASL 口径失真）', ex)
      if (a.some(v => v < 0 || v > 999)) return ERR('关键字需为 0~999 的整数', ex)
      return null
    }
  },

  // ---------- 批2：01_线性表（链表，ListNode 渲染器） ----------
  'singly-linked-insert': {
    title: '单链表按位插入（先接后继）', renderer: 'listNode', steps: singlyLinkedInsertSteps, defaultInput: '3,40',
    parse: (text: string) => { const a = numSeg(text) ?? []; return { raw: a, pos: a[0], value: a[1] } },
    validate: (input: unknown): string | null => {
      const a = (input as { raw: number[] }).raw
      const ex = '3,40'
      if (!Array.isArray(a) || a.length !== 2) return ERR('输入"位序,值"两个整数（演示链表固定为 12,25,33,47,58）', ex)
      const [pos, value] = a
      if (!Number.isInteger(pos) || pos < 1 || pos > 6) return ERR('位序需在 1~6（表长 5，表尾追加即 6）', ex)
      if (!Number.isInteger(value) || value < 1 || value > 999) return ERR('待插值需为 1~999 的整数', ex)
      return null
    }
  },
  'doubly-linked-insert': {
    title: '双链表按位插入（四条指针）', renderer: 'listNode', steps: doublyLinkedInsertSteps, defaultInput: '3,40',
    parse: (text: string) => { const a = numSeg(text) ?? []; return { raw: a, pos: a[0], value: a[1] } },
    validate: (input: unknown): string | null => {
      const a = (input as { raw: number[] }).raw
      const ex = '3,40'
      if (!Array.isArray(a) || a.length !== 2) return ERR('输入"位序,值"两个整数（演示链表固定为 12,25,33,47,58）', ex)
      const [pos, value] = a
      if (!Number.isInteger(pos) || pos < 1 || pos > 6) return ERR('位序需在 1~6（表长 5，表尾追加即 6）', ex)
      if (!Number.isInteger(value) || value < 1 || value > 999) return ERR('待插值需为 1~999 的整数', ex)
      return null
    }
  },
  'josephus-ring': {
    title: '约瑟夫环报数出列', renderer: 'listNode', steps: josephusRingSteps, defaultInput: '8,1,3',
    parse: (text: string) => { const a = numSeg(text) ?? []; return { raw: a, n: a[0], start: a[1], m: a[2] } },
    validate: (input: unknown): string | null => {
      const a = (input as { raw: number[] }).raw
      const ex = '8,1,3'
      if (!Array.isArray(a) || a.length !== 3) return ERR('输入"人数,起始,报数m"三个整数', ex)
      const [n, start, m] = a
      if (!Number.isInteger(n) || n < 2 || n > 9) return ERR('人数需为 2~9 的整数', ex)
      if (!Number.isInteger(start) || start < 1 || start > n) return ERR(`起始位序需在 1~${n}`, ex)
      if (!Number.isInteger(m) || m < 1 || m > 6) return ERR('报数 m 需为 1~6 的整数', ex)
      return null
    }
  },

  // ---------- 批2：02_栈（StackQueue / ListNode 渲染器） ----------
  'seq-stack': {
    title: '顺序栈 push/pop（top 指栈顶元素）', renderer: 'stackQueue', steps: seqStackSteps, defaultInput: 'push:5,push:2,pop,push:7',
    parse: (text: string) => ({ ops: stackOpSeg(text) }),
    validate: (input: unknown): string | null => checkOps((input as { ops: { op: string; v?: number }[] }).ops, 10, 'push:5,push:2,pop,push:7', '"push:值" / "pop"')
  },
  'shared-stack': {
    title: '两栈共享空间（top1/top2 相向增长）', renderer: 'stackQueue', steps: sharedStackSteps, defaultInput: '1:push:5,2:push:9,1:push:3,2:pop',
    parse: (text: string) => ({
      ops: text.split(/[,，\s]+/).filter(Boolean).map(t => {
        const p = /^([12]):push:(-?\d+)$/i.exec(t)
        if (p) return { id: Number(p[1]) as 1 | 2, op: 'push', v: Number(p[2]) }
        const q = /^([12]):pop$/i.exec(t)
        if (q) return { id: Number(q[1]) as 1 | 2, op: 'pop' }
        return { id: 0, op: `看不懂的片段:${t}` }
      })
    }),
    validate: (input: unknown): string | null => {
      const ops = (input as { ops: { id: number; op: string; v?: number }[] }).ops
      const ex = '1:push:5,2:push:9,1:push:3,2:pop'
      if (ops.length < 1) return ERR('至少 1 个操作（"栈号:push:值" / "栈号:pop"）', ex)
      if (ops.length > 10) return ERR('最多 10 个操作', ex)
      if (ops.some(o => o.op !== 'push' && o.op !== 'pop')) return ERR('操作只认"栈号:push:值" / "栈号:pop"（栈号为 1 或 2）', ex)
      if (ops.some(o => o.op === 'push' && (!Number.isInteger(o.v) || (o.v as number) < 1 || (o.v as number) > 999))) return ERR('入栈值需为 1~999 的整数', ex)
      return null
    }
  },
  'link-stack': {
    title: '链栈头插/头删（top 即栈顶）', renderer: 'listNode', steps: linkStackSteps, defaultInput: 'push:5,push:2,push:9,pop,pop',
    parse: (text: string) => ({ ops: stackOpSeg(text) }),
    validate: (input: unknown): string | null => checkOps((input as { ops: { op: string; v?: number }[] }).ops, 10, 'push:5,push:2,push:9,pop,pop', '"push:值" / "pop"')
  },
  'bracket-match': {
    title: '括号匹配（左括号入栈期待配对）', renderer: 'stackQueue', steps: bracketMatchSteps, defaultInput: '{[()]}([)]',
    parse: (text: string) => ({ expr: text.replace(/\s+/g, '') }),
    validate: (input: unknown): string | null => {
      const { expr } = input as { expr: string }
      const ex = '{[()]}([)]'
      if (!expr || expr.length > 16) return ERR('括号串需 1~16 个字符', ex)
      if (/[^()[\]{}]/.test(expr)) return ERR('只能含 ( ) [ ] { } 六种括号字符', ex)
      if ((expr.match(/[([{]/g) ?? []).length > 8) return ERR('左括号个数需 ≤ 8（演示栈容量）', ex)
      return null
    }
  },

  // ---------- 批2：03_队列（StackQueue / ListNode 渲染器） ----------
  'circular-queue': {
    title: '循环顺序队列（牺牲一格判满）', renderer: 'stackQueue', steps: circularQueueSteps, defaultInput: 'en:5,en:2,en:7,de,en:9,en:3,en:4,de',
    parse: (text: string) => ({ ops: queueOpSeg(text) }),
    validate: (input: unknown): string | null => checkOps((input as { ops: { op: string; v?: number }[] }).ops, 12, 'en:5,en:2,de,en:7', '"en:值" / "de"')
  },
  'link-queue': {
    title: '链队列（头结点 + front/rear）', renderer: 'listNode', steps: linkQueueSteps, defaultInput: 'en:5,en:2,de,en:7',
    parse: (text: string) => ({ ops: queueOpSeg(text) }),
    validate: (input: unknown): string | null => checkOps((input as { ops: { op: string; v?: number }[] }).ops, 10, 'en:5,en:2,de,en:7', '"en:值" / "de"')
  },
  'dance-partner': {
    title: '舞伴配对（两队列 FIFO）', renderer: 'stackQueue', steps: dancePartnerSteps, defaultInput: 'F,M,F,F,M,M,F',
    parse: (text: string) => ({
      sexes: text.toUpperCase().split(/[,，\s]+/).filter(Boolean)
    }),
    validate: (input: unknown): string | null => {
      const { sexes } = input as { sexes: string[] }
      const ex = 'F,M,F,F,M,M,F'
      if (sexes.length < 2 || sexes.length > 10) return ERR('需 2~10 位到场者', ex)
      if (sexes.some(s => s !== 'M' && s !== 'F')) return ERR('每位只认 M（男）或 F（女），与 dance_partner.c 的性别标记一致', ex)
      return null
    }
  },

  // ---------- 批3：06_树 / 08_查找（Tree 渲染器，统一树形态） ----------
  'tree-storage-views': {
    title: '树的三种存储结构（双亲/孩子链/孩子兄弟）', renderer: 'tree', steps: (raw: any) => treeStorageViewsSteps({ values: raw }), defaultInput: '1,2,3,4,5',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '1,2,3,4,5'
      if (!Array.isArray(a) || a.length < 1 || a.length > 5) return ERR('需 1~5 个数字（层序数据，按完全树逐层挂父子）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('值需为 1~999 的整数', ex)
      return uniqErr(a, '结点值', ex)
    }
  },
  'seq-bitree-index': {
    title: '二叉树顺序存储编号关系（2i / 2i+1 / ⌊i/2⌋）', renderer: 'tree', steps: (raw: any) => seqBitreeIndexSteps({ values: raw }), defaultInput: '1,2,3,4,5',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '1,2,3,4,5'
      if (!Array.isArray(a) || a.length < 1 || a.length > 7) return ERR('需 1~7 个数字（层序数据，完全树形态）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('值需为 1~999 的整数', ex)
      return uniqErr(a, '结点值', ex)
    }
  },
  'bitree-traverse': {
    title: '二叉树遍历（先/中/后/层序，四模式参数化）', renderer: 'tree', defaultInput: 'pre:A,B,C,#,D,#,#,E,#,#,F,#,#',
    parse: (text: string) => {
      const m = /^([A-Za-z]+)[:：](.*)$/s.exec(text.trim())
      const mode = m ? m[1].toLowerCase() : ''
      const rest = m ? m[2] : text
      return { mode, tokens: rest.split(/[,，\s]+/).filter(Boolean) }
    },
    validate: (input: unknown): string | null => {
      const { mode, tokens } = input as { mode: string; tokens: string[] }
      const ex = 'pre:A,B,C,#,D,#,#,E,#,#,F,#,#'
      if (!['pre', 'in', 'post', 'level'].includes(mode)) return ERR('模式前缀只认 pre: / in: / post: / level:', ex)
      const terr = tokenErr(tokens, ex)
      if (terr) return terr
      if (mode === 'level') {
        const lerr = levelTokenErr(tokens, ex)
        if (lerr) return lerr
        return null
      }
      const built = preNullBuild(tokens)
      if (!built || built.root == null) return ERR('标记串构不成树：# 也要消费，兄弟子树的位置才不会错位', ex)
      return null
    },
    steps: (raw: any) => bitreeTraverseSteps({ mode: raw.mode as TraverseMode, tokens: raw.tokens })
  },
  'thread-build': {
    title: '中序线索化（空链域回收为前驱后继线索）', renderer: 'tree', steps: threadBuildSteps, defaultInput: 'A,B,C,#,D,#,#,E,#,#,F,#,#',
    parse: (text: string) => ({ tokens: text.split(/[,，\s]+/).filter(Boolean) }),
    validate: (input: unknown): string | null => {
      const { tokens } = input as { tokens: string[] }
      const ex = 'A,B,C,#,D,#,#,E,#,#,F,#,#'
      const terr = tokenErr(tokens, ex)
      if (terr) return terr
      const built = preNullBuild(tokens)
      if (!built || built.root == null) return ERR('标记串构不成树：# 也要消费，兄弟子树的位置才不会错位', ex)
      return null
    }
  },
  'forest-convert': {
    title: '森林转二叉树（左孩子右兄弟重排）', renderer: 'tree', steps: forestConvertSteps, defaultInput: '1,2,3;4,5',
    parse: (text: string) => {
      const segsRaw = text.split(/[;；]/).map(s => s.trim())
      return { segsRaw, trees: segsRaw.map(s => numSeg(s) ?? []) }
    },
    validate: (input: unknown): string | null => {
      const { segsRaw, trees } = input as { segsRaw: string[]; trees: number[][] }
      const ex = '1,2,3;4,5'
      if (segsRaw.length < 1 || segsRaw.length > 3) return ERR('需 1~3 棵树（分号分隔，每段一棵树的层序数据）', ex)
      if (trees.some(t => t.length < 1 || t.length > 5)) return ERR('每棵树需 1~5 个数字', ex)
      const all = trees.flat()
      if (all.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('值需为 1~999 的整数', ex)
      return uniqErr(all, '结点值（全森林）', ex)
    }
  },
  'huffman-build': {
    title: '哈夫曼树构造（选两小合并 + WPL）', renderer: 'tree', steps: (raw: any) => huffmanBuildSteps({ weights: raw }), defaultInput: '5,2,9,1,7',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '5,2,9,1,7'
      if (!Array.isArray(a) || a.length < 2 || a.length > 8) return ERR('需 2~8 个权值（正整数）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('权值须为正数（1~999）', ex)
      return null
    }
  },
  'union-find-ops': {
    title: '并查集 union/find（按大小合并 + 路径压缩）', renderer: 'tree', steps: unionFindOpsSteps, defaultInput: 'u:1,2,u:3,4,f:2,u:1,3,f:4',
    parse: (text: string) => {
      // u 的两个参数用逗号写、操作间也用逗号分——用全局正则直接扫描，剩余非空白视为看不懂
      const ops: ({ op: 'u'; a: number; b: number } | { op: 'f'; a: number })[] = []
      const re = /u:(\d+)\s*[,，]\s*(\d+)|f:(\d+)/gi
      let m: RegExpExecArray | null
      while ((m = re.exec(text)) !== null) {
        if (m[3] !== undefined) ops.push({ op: 'f', a: Number(m[3]) })
        else ops.push({ op: 'u', a: Number(m[1]), b: Number(m[2]) })
      }
      const leftovers = text.replace(re, '').replace(/[,，\s]/g, '')
      const elems = ops.flatMap(o => (o.op === 'u' ? [o.a, o.b] : [o.a]))
      return { ops, leftovers, n: elems.length ? Math.max(...elems) : NaN }
    },
    validate: (input: unknown): string | null => {
      const { ops, leftovers, n } = input as { ops: { op: 'u' | 'f'; a: number; b?: number }[]; leftovers: string; n: number }
      const ex = 'u:1,2,u:3,4,f:2,u:1,3,f:4'
      if (leftovers) return ERR(`有看不懂的片段（只认 "u:x,y" 合并 / "f:x" 查找）`, ex)
      if (ops.length < 1 || ops.length > 10) return ERR('需 1~10 个操作', ex)
      if (!Number.isInteger(n) || n < 2 || n > 9) return ERR('元素需为 1~9 的整数（至少出现 2 个不同元素）', ex)
      return null
    }
  },
  'bst-insert-search': {
    title: '二叉排序树插入与查找（比较路径）', renderer: 'tree', steps: bstInsertSearchSteps, defaultInput: '5,2,9,1,7|7',
    parse: (text: string) => {
      const parts = segs(text, 2)
      return { parts, keys: parts ? numSeg(parts[0]) : null, target: parts ? Number(parts[1]) : NaN }
    },
    validate: (input: unknown): string | null => {
      const { parts, keys, target } = input as { parts: string[] | null; keys: number[] | null; target: number }
      const ex = '5,2,9,1,7|7'
      if (!parts) return ERR('格式：插入串|查找目标', ex)
      if (!keys || keys.length < 1 || keys.length > 9) return ERR('插入串需 1~9 个整数', ex)
      if (keys.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('键需为 1~999 的整数', ex)
      const dup = uniqErr(keys, '键', ex)
      if (dup) return dup
      if (!Number.isInteger(target) || target < 1 || target > 999) return ERR('查找目标需为 1~999 的整数', ex)
      return null
    }
  },
  'avl-rotate': {
    title: 'AVL 插入失衡与旋转（LL/LR/RR/RL）', renderer: 'tree', steps: (raw: any) => avlRotateSteps({ keys: raw }), defaultInput: '1,2,3',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '1,2,3'
      if (!Array.isArray(a) || a.length < 3 || a.length > 6) return ERR('需 3~6 个键（validate 会检测序列能否触发旋转）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('键需为 1~999 的整数', ex)
      const dup = uniqErr(a, '键', ex)
      if (dup) return dup
      if (avlRotationCount(a) < 1) return ERR('该序列按 avl.c 插入全程无失衡（如 5,2,9,1 前三键构成满树，第 4 键 BF 仅 ±1）——试能触发旋转的序列：1,2,3 触发 RR、3,2,1 触发 LL、3,1,2 触发 LR、1,3,2 触发 RL', ex)
      return null
    }
  },
  'rb-insert': {
    title: '红黑树插入（叔红变色上推 / 叔黑旋转）', renderer: 'tree', steps: (raw: any) => rbInsertSteps({ keys: raw }), defaultInput: '10,5,20,3',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '10,5,20,3'
      if (!Array.isArray(a) || a.length < 4 || a.length > 8) return ERR('需 4~8 个键（4 个不同键必产生至少一次修复动作）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('键需为 1~999 的整数', ex)
      return uniqErr(a, '键', ex)
    }
  },
  'btree-split': {
    title: 'B 树插入与分裂（t=3，先分裂再下降）', renderer: 'tree', steps: (raw: any) => btreeSplitSteps({ keys: raw }), defaultInput: '10,20,40,50,30,60',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '10,20,40,50,30,60'
      if (!Array.isArray(a) || a.length < 6 || a.length > 8) return ERR('需 6~8 个键（本实现 t=3：单结点至多 2t−1=5 键，第 6 键必触发根分裂）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('键需为 1~999 的整数', ex)
      return uniqErr(a, '键', ex)
    }
  },
  'bplus-insert': {
    title: 'B+ 树插入（叶分裂 + 路由键上浮）', renderer: 'tree', steps: (raw: any) => bplusInsertSteps({ keys: raw }), defaultInput: '10,20,30,40,50,60,70,80,90,15',
    parse: (text: string) => numSeg(text),
    validate: (input: unknown): string | null => {
      const a = input as number[]
      const ex = '10,20,30,40,50,60,70,80,90,15'
      if (a.length < 5 || a.length > 14) return ERR('需 5~14 个键（本实现 t=3：结点上界 2t=6 键，第 6 键触发叶分裂）', ex)
      if (a.some(v => !Number.isInteger(v) || v < 1 || v > 999)) return ERR('键需为 1~999 的整数', ex)
      return uniqErr(a, '键', ex)
    }
  },

  // ---------- 批4：05_串（String 渲染器） ----------
  'string-static-assign': {
    title: '定长顺序串赋值（逐字符拷入 + 超长报错）', renderer: 'string', steps: stringStaticAssignSteps, defaultInput: 'ababcabc',
    parse: (text: string) => ({ src: text.trim() }),
    validate: (input: unknown): string | null => {
      const { src } = input as { src: string }
      const ex = 'ababcabc'
      const err = srcStrErr(src, 1, 12, ex)
      if (err) return err
      return null
    }
  },
  'heap-string-grow': {
    title: '堆分配串动态扩容（满 → 搬家 → free 旧块）', renderer: 'string', steps: heapStringGrowSteps, defaultInput: 'abcdefgh',
    parse: (text: string) => ({ src: text.trim() }),
    validate: (input: unknown): string | null => {
      const { src } = input as { src: string }
      return srcStrErr(src, 5, 8, 'abcdefgh')
    }
  },
  'block-string-chunk': {
    title: '块链串分块存储（CHUNK_SIZE=4 结块）', renderer: 'string', steps: blockStringChunkSteps, defaultInput: 'ababcabcasd',
    parse: (text: string) => ({ src: text.trim() }),
    validate: (input: unknown): string | null => {
      const { src } = input as { src: string }
      return srcStrErr(src, 1, 12, 'ababcabcasd')
    }
  },
  'bf-match': {
    title: '朴素模式匹配 BF（失配回退 i-j+2）', renderer: 'string', steps: bfMatchSteps, defaultInput: 'ababcabcacbab|abcac',
    parse: (text: string) => {
      const parts = segs(text, 2)
      return { parts: parts ?? null, s: parts ? parts[0] : '', t: parts ? parts[1] : '' }
    },
    validate: (input: unknown): string | null => {
      const { parts, s, t } = input as { parts: string[] | null; s: string; t: string }
      const ex = 'ababcabcacbab|abcac'
      if (!parts) return ERR('格式：主串|模式串（用 | 分两段）', ex)
      const se = srcStrErr(s, 1, 14, ex)
      if (se) return se.replace('源串', '主串')
      const te = srcStrErr(t, 1, 5, ex)
      if (te) return te.replace('源串', '模式串')
      if (t.length > s.length) return ERR('模式串不能长于主串（否则一次都比对不齐）', ex)
      return null
    }
  },
  'kmp-match': {
    title: 'KMP 模式匹配（next 表 + i 不回退）', renderer: 'string', steps: kmpMatchSteps, defaultInput: 'ababcabcacbab|abcac',
    parse: (text: string) => {
      const parts = segs(text, 2)
      return { parts: parts ?? null, s: parts ? parts[0] : '', t: parts ? parts[1] : '' }
    },
    validate: (input: unknown): string | null => {
      const { parts, s, t } = input as { parts: string[] | null; s: string; t: string }
      const ex = 'ababcabcacbab|abcac'
      if (!parts) return ERR('格式：主串|模式串（用 | 分两段）', ex)
      const se = srcStrErr(s, 1, 14, ex)
      if (se) return se.replace('源串', '主串')
      const te = srcStrErr(t, 1, 5, ex)
      if (te) return te.replace('源串', '模式串')
      if (t.length > s.length) return ERR('模式串不能长于主串（否则一次都比对不齐）', ex)
      return null
    }
  },

  // ---------- 批4：07_图（Graph 渲染器，环状/分层布局由生成器给定） ----------
  'adjlist-build': {
    title: '邻接表建图（每边对称挂两个弧结点）', renderer: 'graph', steps: adjlistBuildSteps, defaultInput: '0-1,1-2,2-3,0-3|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0-1,1-2,2-3,0-3|4'
      if (!input.parts) return ERR('格式："u-v"边串|顶点数', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      return edgesErr(input.edges, input.n, 12, false, ex)
    }
  },
  'orthlist-build': {
    title: '十字链表建图（一弧一结点挂出/入双链）', renderer: 'graph', steps: orthlistBuildSteps, defaultInput: '0->1,1->2,2->0,1->3|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0->1,1->2,2->0,1->3|4'
      if (!input.parts) return ERR('格式："u->v"有向弧串|顶点数', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      return edgesErr(input.edges, input.n, 10, true, ex)
    }
  },
  'amledg-build': {
    title: '邻接多重表建图（每边一结点两端串接）', renderer: 'graph', steps: amlBuildSteps, defaultInput: '0-1,1-2,2-3,0-2|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0-1,1-2,2-3,0-2|4'
      if (!input.parts) return ERR('格式："u-v"边串|顶点数', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      return edgesErr(input.edges, input.n, 10, false, ex)
    }
  },
  'mst-prim': {
    title: 'Prim 最小生成树（横切最小边入树）', renderer: 'graph', steps: mstPrimSteps, defaultInput: '0-1:4,0-2:1,2-1:2,1-3:5,2-3:8|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0-1:4,0-2:1,2-1:2,1-3:5,2-3:8|4'
      if (!input.parts) return ERR('格式："u-v:w"带权边串|顶点数（无向图）', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      const werr = weightErr(input.edges, ex)
      if (werr) return werr
      return edgesErr(input.edges, input.n, 12, false, ex)
    }
  },
  'mst-kruskal': {
    title: 'Kruskal 最小生成树（排序 + 并查集判环）', renderer: 'graph', steps: mstKruskalSteps, defaultInput: '0-1:1,1-2:2,0-2:3,1-3:4,2-3:5|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0-1:1,1-2:2,0-2:3,1-3:4,2-3:5|4'
      if (!input.parts) return ERR('格式："u-v:w"带权边串|顶点数（无向图）', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      const werr = weightErr(input.edges, ex)
      if (werr) return werr
      return edgesErr(input.edges, input.n, 12, false, ex)
    }
  },
  'sp-dijkstra': {
    title: 'Dijkstra 最短路径（dist 最小定入 + 松弛）', renderer: 'graph', steps: spDijkstraSteps, defaultInput: '0-1:4,0-2:1,2-1:2,1-3:5,2-3:8|4|0',
    parse: (text: string) => parseEdges(text, 3),
    validate: (input: any): string | null => {
      const ex = '0-1:4,0-2:1,2-1:2,1-3:5,2-3:8|4|0'
      if (!input.parts) return ERR('格式："u->v:w"弧串|顶点数|源点（有向网）', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      if (!Number.isInteger(input.src) || input.src < 0 || input.src >= input.n) return ERR(`源点需为 0~${input.n - 1} 的整数`, ex)
      const werr = weightErr(input.edges, ex)
      if (werr) return werr
      return edgesErr(input.edges, input.n, 14, true, ex)
    }
  },
  'topo-kahn': {
    title: '拓扑排序 Kahn（入度削减出队）', renderer: 'graph', steps: topoKahnSteps, defaultInput: '0->1,0->2,2->1,1->3,2->3|4',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0->1,0->2,2->1,1->3,2->3|4'
      if (!input.parts) return ERR('格式："u->v"有向弧串|顶点数（需为 DAG）', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      const err = edgesErr(input.edges, input.n, 12, true, ex)
      if (err) return err
      if (hasCycle(input.edges, input.n)) return ERR('该有向图存在环——拓扑排序要求 DAG，试着去掉构成环的那条弧', ex)
      return null
    }
  },
  'cp-critical-path': {
    title: '关键路径（AOE 网 ve/vl 正逆推）', renderer: 'graph', steps: cpCriticalPathSteps, defaultInput: '0->1:3,0->2:2,1->3:2,2->3:4,3->4:2,3->5:3,4->5:1|6',
    parse: (text: string) => parseEdges(text, 2),
    validate: (input: any): string | null => {
      const ex = '0->1:3,0->2:2,1->3:2,2->3:4,3->4:2,3->5:3,4->5:1|6'
      if (!input.parts) return ERR('格式："u->v:w"活动弧串|顶点数（AOE 网，须单源单汇无环）', ex)
      if (!Number.isInteger(input.n) || input.n < 2 || input.n > 8) return ERR('顶点数需为 2~8 的整数', ex)
      const werr = weightErr(input.edges, ex, 1)
      if (werr) return werr
      const err = edgesErr(input.edges, input.n, 12, true, ex)
      if (err) return err
      if (hasCycle(input.edges, input.n)) return ERR('AOE 网不允许成环（工程排不出工期），去掉构成环的弧', ex)
      const indeg = Array.from({ length: input.n }, () => 0)
      const outdeg = Array.from({ length: input.n }, () => 0)
      for (const e of input.edges) { indeg[e.v]++; outdeg[e.u]++ }
      const srcs = indeg.filter((d: number) => d === 0).length
      const dsts = outdeg.filter((d: number) => d === 0).length
      if (srcs !== 1 || dsts !== 1) return ERR(`源点（入度 0）与汇点（出度 0）各须恰 1 个，当前 ${srcs}/${dsts} 个`, ex)
      return null
    }
  },
  'crosslist-sparse': {
    title: '稀疏矩阵十字链表（行链列链双有序）', renderer: 'graph', steps: crosslistSparseSteps, defaultInput: '0-1:5,0-3:2,2-1:4|3|4',
    parse: (text: string) => {
      const parts = segs(text, 3)
      if (!parts) return { parts: null as string[] | null, cells: [] as { i: number; j: number; v: number }[], rows: NaN, cols: NaN }
      const cells = parts[0].split(/[,，\s]+/).filter(Boolean).map(edgeTok).map(t => t ?? { u: NaN, v: NaN, w: NaN })
        .map(t => ({ i: t.u, j: t.v, v: t.w as number }))
      return { parts, cells, rows: Number(parts[1]), cols: Number(parts[2]) }
    },
    validate: (input: any): string | null => {
      const ex = '0-1:5,0-3:2,2-1:4|3|4'
      if (!input.parts) return ERR('格式："行-列:值"串|行数|列数', ex)
      const { cells, rows, cols } = input
      if (!Number.isInteger(rows) || rows < 2 || rows > 5) return ERR('行数需为 2~5 的整数', ex)
      if (!Number.isInteger(cols) || cols < 2 || cols > 5) return ERR('列数需为 2~5 的整数', ex)
      if (cells.length < 1 || cells.length > 8) return ERR('非零元需 1~8 个', ex)
      if (cells.some((c: any) => !Number.isInteger(c.i) || !Number.isInteger(c.j) || !Number.isInteger(c.v))) return ERR('非零元需为"行-列:值"三个整数', ex)
      if (cells.some((c: any) => c.i < 0 || c.i >= rows || c.j < 0 || c.j >= cols)) return ERR(`行号 0~${rows - 1}、列号 0~${cols - 1}`, ex)
      if (cells.some((c: any) => c.v === 0)) return ERR('非零元的值不能为 0（零元不入链）', ex)
      const seen = new Set<string>()
      for (const c of cells) {
        const key = `${c.i}-${c.j}`
        if (seen.has(key)) return ERR('同一位置重复设置', ex)
        seen.add(key)
      }
      return null
    }
  },

  // ---------- 批4：09_排序/外部排序（String 渲染器多行） ----------
  'external-merge': {
    title: '外部 k 路归并（败者树选段首最小）', renderer: 'string', steps: externalMergeSteps, defaultInput: '1,3,5|2,4,6|0,7',
    parse: (text: string) => {
      const parts = text.split('|').map(s => s.trim())
      return { parts, segsRaw: parts, segs: parts.map(p => numSeg(p) ?? [NaN]) }
    },
    validate: (input: any): string | null => {
      const ex = '1,3,5|2,4,6|0,7'
      if (input.parts.length < 2 || input.parts.length > 4 || input.parts.some((p: string) => !p.length)) return ERR('格式：2~4 个有序段用 | 分隔（段内逗号分隔数字）', ex)
      const { segs } = input
      if (segs.some((s: number[]) => s.length < 1 || s.length > 8)) return ERR('每段需 1~8 个数字', ex)
      if (segs.some((s: number[]) => s.some((v: number) => !Number.isInteger(v) || v < 0 || v > 999))) return ERR('值需为 0~999 的整数', ex)
      for (const s of segs) {
        for (let i = 1; i < s.length; i++) {
          if (s[i] < s[i - 1]) return ERR('每段自身需升序（外部归并的前提：初等运行已各自排好）', ex)
        }
      }
      if (segs.reduce((t: number, s: number[]) => t + s.length, 0) > 20) return ERR('总记录数需 ≤ 20（演示规模）', ex)
      return null
    }
  }
}
