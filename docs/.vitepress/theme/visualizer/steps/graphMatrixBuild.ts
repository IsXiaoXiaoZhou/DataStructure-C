// docs/.vitepress/theme/visualizer/steps/graphMatrixBuild.ts
import type { Step } from '../types'

// 邻接矩阵建图（对应 07_图/01_邻接矩阵 adjacency_matrix.c 的 mg_set_edge）
// 语义：n×n 矩阵 matrix[u][v] 存边标记；无向图加边对称双写保持对称不变量，
// 有向图只写一处；state 用桶形态：buckets = n 行（矩阵行），bucketLabels = 顶点号
export interface GraphEdge { u: number; v: number }
export interface GraphMatrixBuildInput { edges: GraphEdge[]; n: number; directed: boolean }

export function graphMatrixBuildSteps(input: GraphMatrixBuildInput): Step[] {
  const { edges, n, directed } = input
  const m: number[][] = Array.from({ length: n }, () => new Array(n).fill(0))
  const labels = Array.from({ length: n }, (_, i) => `v${i}`)
  const frame = (narration: string): Step =>
    ({ state: { array: [], buckets: m.map(r => [...r]), bucketLabels: labels }, highlights: [], active: null, narration })

  const steps: Step[] = [frame(
    `${directed ? '有向' : '无向'}图，n = ${n} 个顶点：邻接矩阵从全 0 开始（本模块是"网"模型，无边实际存 INF 哨兵，动画里以 0/1 示意）；${directed ? '有向边只写一处' : '无向边要对称写两处'}`)]

  for (const e of edges) {
    m[e.u][e.v] = 1
    steps.push(frame(`加边 ${e.u}-${e.v}：matrix[${e.u}][${e.v}] = 1（行 u、列 v 的交叉点落子）`))
    if (!directed) {
      m[e.v][e.u] = 1
      steps.push(frame(`无向图对称写：matrix[${e.v}][${e.u}] = 1——对称不变量在加边时就该保持，漏一处矩阵就不再对称`))
    }
  }

  const deg = m.map((row, i) => row.reduce((s, x) => s + x, 0))
  steps.push(frame(
    `建图完成：${n} 个顶点 ${edges.length} 条边；判边 O(1)——直接看 matrix[u][v]，代价是 O(n²) 空间、找邻居要扫整行（v0 度 ${deg[0]}，v${n - 1} 度 ${deg[n - 1]}）`))

  return steps
}
