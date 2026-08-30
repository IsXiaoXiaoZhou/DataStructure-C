// docs/.vitepress/theme/visualizer/steps/amlBuild.ts
import type { Step } from '../types'
import { gframe, ringCoords, ukey, type GraphBuildInput } from './graphKit'

// 邻接多重表建图（对应 07_图/04_邻接多重表 adjacency_multilist.c 的 aml_set_edge）
// 源码语义：无向图每条边只开一个边结点 [ivex|jvex|ilink|jlink|weight]，同时头插
// 进两端的依附链；遍历某顶点的链要"看路牌走链"：ivex==v 沿 ilink，否则沿 jlink。
export function amlBuildSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  const chains: { u: number; v: number }[][] = Array.from({ length: n }, () => []) // first_edge[v]（头插序）
  const nodes = () => coords.map((c, i) => ({ id: i, label: `v${i}`, x: c.x, y: c.y }))
  const drawn = (upto: number, hot?: { u: number; v: number }) => edges.slice(0, upto + 1).map(e => ({
    from: e.u, to: e.v,
    active: !!hot && ukey(hot.u, hot.v) === ukey(e.u, e.v)
  }))
  const aux = () => chains.map((chain, i) => ({
    text: `first_edge[${i}] → ${chain.length ? chain.map(e => `e(${e.u},${e.v})`).join('—') + ' → NULL' : 'NULL'}`
  }))

  const steps: Step[] = [gframe(nodes(), [], aux(),
    `邻接多重表开建：与邻接表"一条无向边存两个弧结点"不同——每条边只开一个结点 [ivex|jvex|ilink|jlink|weight]，同时挂在 ivex、jvex 两端的依附链上，删边只需 free 一次`)]

  edges.forEach((e, ei) => {
    const u = e.u, v = e.v
    chains[u].unshift({ u, v })  // 头插进 u 的依附链（ilink 方向）
    chains[v].unshift({ u, v })  // 头插进 v 的依附链（jlink 方向）
    steps.push(gframe(nodes(), drawn(ei, { u, v }), aux(),
      `加边 e(${u},${v})：一个新边结点两次头插——edge->ilink = first_edge[${u}] 后 first_edge[${u}] = edge，edge->jlink = first_edge[${v}] 后 first_edge[${v}] = edge（第 ${ei + 1}/${edges.length} 条边，边结点数 ${ei + 1} 而非邻接表的 ${2 * (ei + 1)}）`))
  })

  steps.push(gframe(nodes(), edges.map(e => ({ from: e.u, to: e.v })), aux(),
    `建图完成：n 个链头 + ${edges.length} 个边结点（邻接表要 2e = ${2 * edges.length} 个）；遍历顶点 v 的依附链要"看路牌走链"：next_of(e, v) = e->ivex == v ? ilink : jlink——每走一步先判断当前边哪端是 v`))

  return steps
}
