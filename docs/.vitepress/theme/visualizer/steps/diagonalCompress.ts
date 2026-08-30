// docs/.vitepress/theme/visualizer/steps/diagonalCompress.ts
import type { Step } from '../types'

// 三对角矩阵压缩（对应 04_特殊矩阵压缩存储/03_对角矩阵 tridiagonal_matrix.c）
// 语义：仅 |i-j| <= 1 的元素非零，按行序压入长度 3n-2 的一维数组
//   k = 2i + j - 3（前 i-1 行共 3i-4 个 + 行内偏移 j-i+1）
export function diagonalCompressSteps(input: number[]): Step[] {
  const L = input.length
  const n = (L + 2) / 3
  const data: number[] = new Array(L).fill(0)
  const rows: number[][] = Array.from({ length: n }, () => [])
  const labels = Array.from({ length: n }, (_, i) => `第${i + 1}行`)
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...data], buckets: rows.map(r => [...r]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `${n} 阶三对角矩阵：非零元只有三条对角线，共 3×${n}−2 = ${L} 个，按行序压入；映射 k = 2i + j − 3（首行 2 个、中间行各 3 个、末行 2 个）`)]

  let t = 0
  for (let i = 1; i <= n; i++) {
    for (let j = Math.max(1, i - 1); j <= Math.min(n, i + 1); j++) {
      const v = input[t++]
      const k = 2 * i + j - 3
      data[k] = v
      rows[i - 1].push(v)
      // 3i-4（前 i-1 行个数）仅对 i ≥ 2 成立；第 1 行之前没有行、行内从 a(1,1) 起存
      const prevNote = i === 1
        ? `第 1 行之前没有行（前面共 0 个），行内偏移 ${j}−1`
        : `前 ${i - 1} 行共 ${3 * i - 4} 个，行内偏移 ${j}−(${i - 1})`
      steps.push(frame(
        `a(${i},${j}) = ${v}：k = 2×${i} + ${j} − 3 = ${k}，放入 data[${k}]（${prevNote}）`,
        [k], [k, k]))
    }
  }

  steps.push(frame(
    `压缩完成：[${data.join(', ')}]，3n−2 个单元恰好装下 2+3(${n - 2})+2 个带内元素；带外是真 0——与三角矩阵的常数 c 不同，set 带外非零值会被直接拒绝`,
    data.map((_, k) => k), null))

  return steps
}
