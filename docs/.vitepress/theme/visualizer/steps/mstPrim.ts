// docs/.vitepress/theme/visualizer/steps/mstPrim.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// Prim 最小生成树（对应 07_图/05_最小生成树 min_spanning_tree.c 的 mst_prim）
// 源码语义：lowcost 初值取第 0 行，lowcost = -1 标已入选、INF 表不可达/树外无边；
// 每轮选树外 lowcost 最小点 best（并列取编号小者），入选后用新点松弛所有树外点
// （g->w[best][v] < lowcost[v] 才更新，nearest 同步），共 n-1 轮；选不出点即不连通。
const INF = '∞'

export function mstPrimSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  // 无向带权：邻接权表（后写覆盖先写，与 mstg_add_edge 对称双写一致）
  const w: Record<string, number> = {}
  for (const e of edges) {
    w[`${e.u}-${e.v}`] = e.w ?? 1
    w[`${e.v}-${e.u}`] = e.w ?? 1
  }
  const wt = (a: number, b: number) => w[`${a}-${b}`]

  const allEdges = () => edges.map(e => ({ from: e.u, to: e.v, weight: e.w ?? 1 }))
  const treeEdges: { u: number; v: number; w: number }[] = []
  const edgesView = (active?: { a: number; b: number }) => allEdges().map(e => ({
    from: e.from, to: e.to, weight: e.weight,
    highlight: treeEdges.some(t => (t.u === e.from && t.v === e.to) || (t.u === e.to && t.v === e.from)),
    active: !!active && ((active.a === e.from && active.b === e.to) || (active.a === e.to && active.b === e.from))
  }))
  const nodesView = (inTree: boolean[], active?: number) => coords.map((c, i) => ({
    id: i, label: `v${i}`, x: c.x, y: c.y,
    highlight: inTree[i], active: active === i
  }))
  const lcTxt = (v: number) => (inTree[v] ? '−' : (lowcost[v] === Infinity ? INF : String(lowcost[v])))
  let lowcost: number[] = []
  let nearest: number[] = []
  let inTree: boolean[] = []
  const aux = () => [
    { text: `lowcost: [${Array.from({ length: n }, (_, i) => lcTxt(i)).join(', ')}]  （− = 已入选，∞ = 树外暂无边）` },
    { text: `nearest: [${Array.from({ length: n }, (_, i) => (i === 0 ? '−' : String(nearest[i]))).join(', ')}]  已入选: {${inTree.map((t, i) => (t ? i : null)).filter(x => x !== null).join(',')}}  树边: ${treeEdges.length}/${n - 1}` }
  ]

  // 初始化：lowcost 取第 0 行
  lowcost = Array.from({ length: n }, (_, v) => (wt(0, v) ?? Infinity))
  nearest = Array.from({ length: n }, () => 0)
  inTree = Array.from({ length: n }, () => false)
  inTree[0] = true
  lowcost[0] = -1

  const steps: Step[] = [gframe(nodesView(inTree), edgesView(), aux(),
    `Prim 从 0 号点起步：lowcost[v] 初值取第 0 行 w[0][v]（∞ = 暂无边），nearest[v] = 0；lowcost = -1 标已入选——每轮把"横切边"最短的树外点拉进树，O(n²)`)]

  let total = 0
  for (let k = 0; k + 1 < n; k++) {
    // 选树外 lowcost 最小的点
    let best = -1, bestW = Infinity
    for (let v = 1; v < n; v++) {
      if (!inTree[v] && lowcost[v] > 0 && lowcost[v] < bestW) { bestW = lowcost[v]; best = v }
    }
    if (best === -1) {
      steps.push(gframe(nodesView(inTree), edgesView(), aux(),
        `选不出可入选点（树外 lowcost 全为 ∞）：图不连通，无法生成 MST（源码返回 DS_ERROR）——已选 ${treeEdges.length} 条即止`))
      return steps
    }
    treeEdges.push({ u: nearest[best], v: best, w: bestW })
    total += bestW
    steps.push(gframe(nodesView(inTree, best), edgesView({ a: nearest[best], b: best }), aux(),
      `第 ${k + 1} 轮选边：横切边候选比较后最小者 ${nearest[best]}-${best}（权 ${bestW}）——把 v${best} 拉进树（总权累计 ${total}）${k + 2 === n ? '；凑满 n-1 条' : ''}`))
    inTree[best] = true
    lowcost[best] = -1
    // 用新点松弛所有树外点
    for (let v = 1; v < n; v++) {
      if (!inTree[v] && wt(best, v) !== undefined && wt(best, v)! < lowcost[v]) {
        const old = lowcost[v] === Infinity ? INF : String(lowcost[v])
        lowcost[v] = wt(best, v)!
        nearest[v] = best
        steps.push(gframe(nodesView(inTree), edgesView({ a: best, b: v }), aux(),
          `松弛：lowcost[${v}] = min(${old}, w[${best}][${v}] = ${wt(best, v)}) = ${lowcost[v]}，nearest[${v}] 改记 ${best}——横切边集合随新点更新`))
      }
    }
  }

  steps.push(gframe(nodesView(inTree), edgesView(), aux(),
    `Prim 完成：${n - 1} 条树边总权 = ${total}（MST 权值唯一，树形态遇等权边可能不唯一）；换起点总权不变——手算对答案报总权就行`))

  return steps
}
