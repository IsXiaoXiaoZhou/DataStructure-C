// docs/.vitepress/theme/visualizer/steps/adjlistBuild.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// 邻接表建图（对应 07_图/02_邻接表 adjacency_list.c 的 ag_set_edge + append_arc）
// 源码语义：无向图加边 = 对称挂两个弧结点（2e 个结点）；append_arc 是"扫到链尾
// 再挂"的尾插，链序即插入序（教材常见写法是新结点头插，链序为插入逆序）。
export function adjlistBuildSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  const first: number[][] = Array.from({ length: n }, () => [])   // first[u] 邻接链（尾插序）
  const nodes = () => coords.map((c, i) => ({ id: i, label: `v${i}`, x: c.x, y: c.y }))
  const drawn = (upto: number, hot?: { a: number; b: number }) => edges.slice(0, upto + 1).map(e => ({
    from: e.u, to: e.v,
    active: !!hot && ((hot.a === e.u && hot.b === e.v) || (hot.a === e.v && hot.b === e.u))
  }))
  const aux = () => first.map((chain, i) => ({
    text: `first[${i}] → ${chain.length ? chain.map(v => `(${v})`).join('→') + ' → NULL' : 'NULL'}`
  }))

  const steps: Step[] = [gframe(nodes(), [], aux(),
    `邻接表开建：n=${n} 个顶点的 first[u] 头指针数组全为 NULL（环状布局仅为演示）。每条无向边要对称挂两个弧结点 [adjvex|weight|next]——e 条边共 2e 个结点`)]

  let arcs = 0
  edges.forEach((e, ei) => {
    first[e.u].push(e.v)
    arcs++
    steps.push(gframe(nodes(), drawn(ei, { a: e.u, b: e.v }), aux(),
      `边 ${e.u}-${e.v} 第 1 个弧结点：append_arc 尾插进 first[${e.u}] 链尾——本实现"扫到链尾再挂"保插入序（教材头插写法链序恰为插入逆序），已挂 ${arcs} 个弧结点`))
    first[e.v].push(e.u)
    arcs++
    steps.push(gframe(nodes(), drawn(ei, { a: e.u, b: e.v }), aux(),
      `无向边对称挂第 2 个：${e.u}-${e.v} 也进 first[${e.v}] 链——漏挂这一侧，v 的邻居枚举就缺一条边；累计 ${arcs} = 2×${edges.length} 个弧结点`))
  })

  const deg = first.map(c => c.length)
  steps.push(gframe(nodes(), edges.map(e => ({ from: e.u, to: e.v })), aux(),
    `建图完成：n=${n} 个头指针 + ${arcs} 个弧结点（无向图 2e），空间 O(n+e)；顶点 u 的度 = first[u] 链长（v0 度 ${deg[0]}）；判边要沿链扫 O(deg)——邻接矩阵 O(1) 判边、邻接表省空间，各有取舍`))

  return steps
}
