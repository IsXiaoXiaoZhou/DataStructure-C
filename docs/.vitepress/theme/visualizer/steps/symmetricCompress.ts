// docs/.vitepress/theme/visualizer/steps/symmetricCompress.ts
import type { Step } from '../types'

// 对称矩阵下三角压缩映射（对应 04_特殊矩阵压缩存储/01_对称矩阵 symmetric_matrix.c）
// 语义：只存下三角（含主对角线），1-based 行主序压入一维数组
//   i >= j: k = i(i-1)/2 + j - 1；输入按行主序给出下三角元素
export function symmetricCompressSteps(input: number[]): Step[] {
  const L = input.length
  const n = (Math.sqrt(8 * L + 1) - 1) / 2
  const data: number[] = new Array(L).fill(0)
  const rows: number[][] = Array.from({ length: n }, () => [])
  const labels = Array.from({ length: n }, (_, i) => `第${i + 1}行`)
  const frame = (narration: string, highlights: number[] = [], active: number[] | null = null): Step =>
    ({ state: { array: [...data], buckets: rows.map(r => [...r]), bucketLabels: labels }, highlights, active, narration })

  const steps: Step[] = [frame(
    `${n} 阶对称矩阵，只存下三角 ${L} = ${n}(${n}+1)/2 个元素（含主对角线），一维数组先全部空着；映射公式 k=i(i−1)/2+j−1，行列号从 1 计`,
    [], null)]

  let t = 0
  for (let i = 1; i <= n; i++) {
    for (let j = 1; j <= i; j++) {
      const v = input[t++]
      const k = (i * (i - 1)) / 2 + j - 1
      data[k] = v
      rows[i - 1].push(v)
      steps.push(frame(
        `a(${i},${j}) = ${v}：k = ${i}×${i - 1}/2 + ${j} − 1 = ${k}，放入 data[${k}]（第 i 行之前已有 1+2+…+${i - 1} 个）`,
        [k], [k, k]))
    }
  }

  steps.push(frame(
    `压缩完成：[${data.join(', ')}]；任取 a(i,j)，i<j 时交换行列后查同一单元——上/下三角共享存储，对称性在存储层面就不可能被破坏`,
    data.map((_, k) => k), null))

  return steps
}
