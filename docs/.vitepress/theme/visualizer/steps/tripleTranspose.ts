// docs/.vitepress/theme/visualizer/steps/tripleTranspose.ts
import type { Step } from '../types'

// 稀疏矩阵快速转置（对应 04_特殊矩阵压缩存储/04_稀疏矩阵-三元组 sparse_matrix.c）
// 语义：原表按行序存储；第一遍统计每列个数 num[c]，递推各列首元落位
//   cpot[0] = 0，cpot[c] = cpot[c-1] + num[c-1]；第二遍扫描按 cpot
//   直接落位，落一个自增一次。两遍共 O(nu + tu)
export interface Triple { r: number; c: number; v: number }
export interface TripleTransposeInput { triples: Triple[]; cols: number }

export function tripleTransposeSteps(input: TripleTransposeInput): Step[] {
  const { triples, cols } = input
  const tu = triples.length
  const num: number[] = new Array(cols).fill(0)
  const cpot: number[] = new Array(cols).fill(0)
  const out: number[] = new Array(tu).fill(0)
  const buckets = () => [num.slice(), cpot.slice()]
  const labels = ['num（各列个数）', 'cpot（列首元落位）']
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...out], buckets: buckets(), bucketLabels: labels }, highlights, active, narration })

  const src = triples.map(t => `(${t.r},${t.c},${t.v})`).join(' ')
  const steps: Step[] = [frame(
    `原表 ${tu} 个三元组（按行序）：${src}，列数 nu = ${cols}；转置后要按新行序（= 原列序）排列——快速转置先算 num/cpot，一遍扫描直接定位`)]

  for (const t of triples) {
    num[t.c - 1]++
    steps.push(frame(
      `扫到 (${t.r},${t.c},${t.v})：列 ${t.c} 的 num[${t.c - 1}] 增到 ${num[t.c - 1]}（第一遍只计数，不搬数据）`,
      [], null))
  }
  steps.push(frame(
    `num 计完：[${num.join(', ')}]；接下来递推 cpot——cpot[0] = 0，cpot[c] = cpot[c−1] + num[c−1]`))

  cpot[0] = 0
  for (let c = 1; c < cols; c++) {
    cpot[c] = cpot[c - 1] + num[c - 1]
    steps.push(frame(
      `cpot[${c}] = cpot[${c - 1}] + num[${c - 1}] = ${cpot[c - 1]} + ${num[c - 1]} = ${cpot[c]}（第 ${c + 1} 列首元在结果表中的下标）`,
      [], null))
  }

  for (const t of triples) {
    const q = cpot[t.c - 1]++
    out[q] = t.v
    steps.push(frame(
      `(${t.r},${t.c},${t.v}) 转置成 (${t.c},${t.r},${t.v})：q = cpot[${t.c - 1}] = ${q} → 转置表[${q}] = ${t.v}，cpot[${t.c - 1}] 自增为同列下一个让位`,
      [q], [q, q]))
  }

  steps.push(frame(
    `快速转置完成：转置表 [${out.join(', ')}] 对应 ${triples.map(t => `(${t.c},${t.r},${t.v})`).join(' ')}；两遍扫描 O(nu+tu)，对比按列逐趟扫描的 O(nu·tu)——用两个辅助数组换掉重扫`,
    out.map((_, k) => k), null))

  return steps
}
