// docs/.vitepress/theme/visualizer/steps/orthlistBuild.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// 十字链表建图（对应 07_图/03_十字链表 orthogonal_list.c 的 og_add_arc）
// 源码语义：有向图每条弧恰一个结点 [tailvex|headvex|tlink|hlink|weight]，
// 新弧头插进弧尾的出弧链（tlink）与弧头的入弧链（hlink）——双链共享结点。
export function orthlistBuildSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  const outChains: number[][] = Array.from({ length: n }, () => []) // first_out[u]（头插序）
  const inChains: number[][] = Array.from({ length: n }, () => [])  // first_in[v]（头插序）
  const nodes = () => coords.map((c, i) => ({ id: i, label: `v${i}`, x: c.x, y: c.y }))
  const drawn = (upto: number, hot?: { a: number; b: number }) => edges.slice(0, upto + 1).map(e => ({
    from: e.u, to: e.v, directed: true,
    active: !!hot && hot.a === e.u && hot.b === e.v
  }))
  const chainTxt = (chain: number[], other: (x: number) => number) =>
    chain.length ? chain.map(x => `(→v${other(x)})`).join('→') + ' → NULL' : 'NULL'
  const aux = () => outChains.flatMap((oc, i) => [{
    text: `v${i}  出弧链 first_out → ${chainTxt(oc, x => x)}   入弧链 first_in → ${chainTxt(inChains[i], x => x)}`
  }])

  const steps: Step[] = [gframe(nodes(), [], aux(),
    `十字链表开建：有向图每条弧只开一个结点 [tailvex|headvex|hlink|tlink]，同时挂在两条链上——弧尾 u 的出弧链（tlink 串）与弧头 v 的入弧链（hlink 串），"十字"由此得名`)]

  edges.forEach((e, ei) => {
    outChains[e.u].unshift(e.v)  // 头插：新弧成为链头（链序 = 插入逆序）
    inChains[e.v].unshift(e.u)
    steps.push(gframe(nodes(), drawn(ei, { a: e.u, b: e.v }), aux(),
      `加弧 u${e.u}→v${e.v}：新结点 tailvex=${e.u}、headvex=${e.v} 头插两处——arc->tlink = first_out[${e.u}] 后 first_out[${e.u}] = arc，arc->hlink = first_in[${e.v}] 后 first_in[${e.v}] = arc，一次 malloc 两根针（第 ${ei + 1}/${edges.length} 条弧）`))
  })

  const arcCount = edges.length
  steps.push(gframe(nodes(), edges.map(e => ({ from: e.u, to: e.v, directed: true })), aux(),
    `建图完成：${n} 对链头 + ${arcCount} 个弧结点（对比邻接表存有向图同为 e 个结点，但十字链表的入弧链免费送：求入度沿 first_in 走即可，不必像出边邻接表那样全表扫描 O(n+e)）`))

  return steps
}
