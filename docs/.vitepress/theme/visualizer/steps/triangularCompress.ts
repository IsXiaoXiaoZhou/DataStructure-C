// docs/.vitepress/theme/visualizer/steps/triangularCompress.ts
import type { Step } from '../types'

// 三角矩阵（下三角 + 常数 c）压缩（对应 04_特殊矩阵压缩存储/02_三角矩阵 triangular_matrix.c）
// 语义：只存下三角 n(n+1)/2 个元素，末位单独放常数 c，共 n(n+1)/2+1 个单元
//   下三角 i >= j: k = i(i-1)/2 + j - 1；带外（i<j）一律映射到末位读 c
export interface TriangularCompressInput { values: number[]; c: number }

export function triangularCompressSteps(input: TriangularCompressInput): Step[] {
  const { values, c } = input
  const L = values.length
  const n = (Math.sqrt(8 * L + 1) - 1) / 2
  const data: number[] = new Array(L + 1).fill(0)
  const rows: number[][] = Array.from({ length: n + 1 }, () => [])
  const labels = [...Array.from({ length: n }, (_, i) => `第${i + 1}行`), `常数 c`]
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...data], buckets: rows.map(r => [...r]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `${n} 阶下三角矩阵：三角区 ${L} 个元素 + 末位 1 个常数 c，共 ${L + 1} = ${n}(${n}+1)/2 + 1 个单元；带外元素不存储，统一读 c`)]

  let t = 0
  for (let i = 1; i <= n; i++) {
    for (let j = 1; j <= i; j++) {
      const v = values[t++]
      const k = (i * (i - 1)) / 2 + j - 1
      data[k] = v
      rows[i - 1].push(v)
      steps.push(frame(
        `a(${i},${j}) = ${v}（下三角带内）：k = ${i}×${i - 1}/2 + ${j} − 1 = ${k}，放入 data[${k}]`,
        [k], [k, k]))
    }
  }

  data[L] = c
  rows[n].push(c)
  steps.push(frame(
    `带外（i<j 的所有 a(i,j)）统一是常数 c = ${c}：独占末位 data[${L}]——查 a(1,2) 返回的就是它，与对称矩阵"共享存储"不同，上三角不占格子`,
    [L], [L, L]))

  steps.push(frame(
    `压缩完成：[${data.join(', ')}]，data 长 ${L + 1}；三角区走 k = i(i-1)/2 + j − 1，带外走末位——带外全为 c 这一矩阵语义在存储层完整保留`,
    data.map((_, k) => k), null))

  return steps
}
