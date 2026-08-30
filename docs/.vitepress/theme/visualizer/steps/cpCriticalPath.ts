// docs/.vitepress/theme/visualizer/steps/cpCriticalPath.ts
import type { Step } from '../types'
import { gframe, ringCoords } from './graphKit'

// 关键路径（对应 07_图/08_关键路径 critical_path.c 的 cp_analyze，AOE 网）
// 源码语义：单源单汇检查 → Kahn 拓扑保序 → 正推 ve（沿拓扑序取 max）→
// 逆推 vl（vl[汇] = ve[汇]，逆序取 min）→ 活动表 e = ve[u]、l = vl[v] - w，
// e == l 的活动是关键活动；duration = ve[汇]。布局：分层拓扑坐标（层内均分）。
export interface CpInput { edges: { u: number; v: number; w: number }[]; n: number }

export function cpCriticalPathSteps(input: CpInput): Step[] {
  const { edges, n } = input
  const indeg: number[] = Array.from({ length: n }, () => 0)
  const outdeg: number[] = Array.from({ length: n }, () => 0)
  for (const e of edges) { indeg[e.v]++; outdeg[e.u]++ }

  // 源汇识别（单源单汇，源码按错误处理多源/多汇）
  const srcs = indeg.map((d, i) => (d === 0 ? i : -1)).filter(i => i >= 0)
  const dsts = outdeg.map((d, i) => (d === 0 ? i : -1)).filter(i => i >= 0)
  if (srcs.length !== 1 || dsts.length !== 1) {
    const coords0 = ringCoords(n)
    return [gframe(coords0.map((c, i) => ({ id: i, label: `v${i}`, x: c.x, y: c.y })),
      edges.map(e => ({ from: e.u, to: e.v, weight: e.w, directed: true })), [],
      `AOE 网前置检查失败：源点 ${srcs.length} 个、汇点 ${dsts.length} 个（各须恰 1）——源码返回 DS_ERROR（多源/多汇在真实工程应加超源超汇）`)]
  }

  // Kahn 拓扑（deg=−1 标已输出，与源码一致）
  const deg = [...indeg]
  const topo: number[] = []
  for (let k = 0; k < n; k++) {
    const pick = deg.findIndex(d => d === 0)
    if (pick === -1) {
      const coords0 = ringCoords(n)
      return [gframe(coords0.map((c, i) => ({ id: i, label: `v${i}`, x: c.x, y: c.y })),
        edges.map(e => ({ from: e.u, to: e.v, weight: e.w, directed: true })), [],
        `存在有向环（找不到 0 入度点）：AOE 网不允许成环，工程根本排不出工期（DS_ERROR）`)]
    }
    deg[pick] = -1
    topo.push(pick)
    for (const e of edges) if (e.u === pick && deg[e.v] !== -1) deg[e.v]--
  }

  // 分层拓扑布局：事件画在所属拓扑层（层号 = 最长路径意义下的深度）
  const layer = Array.from({ length: n }, () => 0)
  for (const u of topo) for (const e of edges) if (e.u === u) layer[e.v] = Math.max(layer[e.v], layer[u] + 1)
  const maxLayer = Math.max(...layer)
  const colCount = Array.from({ length: maxLayer + 1 }, () => 0)
  const colIdx = Array.from({ length: n }, () => 0)
  for (const u of topo) colIdx[u] = colCount[layer[u]]++
  const nodePos = (i: number) => ({
    x: 12 + (76 * (colIdx[i] + (colCount[layer[i]] === 1 ? 0.5 : 0))) / Math.max(1, colCount[layer[i]]),
    y: 14 + (72 * layer[i]) / Math.max(1, maxLayer)
  })
  const nodesOf = (marks?: { hl?: number[]; act?: number }) => Array.from({ length: n }, (_, i) => ({
    id: i, label: `v${i}`, x: +nodePos(i).x.toFixed(2), y: +nodePos(i).y.toFixed(2),
    highlight: marks?.hl?.includes(i), active: marks?.act === i
  }))
  const edgesOf = (crit?: Set<string>, act?: { u: number; v: number }) => edges.map(e => ({
    from: e.u, to: e.v, weight: e.w, directed: true,
    highlight: crit?.has(`${e.u}-${e.v}`),
    active: !!act && act.u === e.u && act.v === e.v
  }))

  // 正推 ve
  const ve: number[] = Array.from({ length: n }, () => 0)
  const vl: number[] = Array.from({ length: n }, () => 0)
  const dst = dsts[0]
  const veAux = () => [{ text: `ve: [${ve.join(', ')}]   vl: [${vl.join(', ')}]   汇点 v${dst}，工期 = ve[汇] = ${ve[dst]}` }]
  const steps: Step[] = [gframe(nodesOf(), edgesOf(), veAux(),
    `AOE 网：顶点=事件（入度 0 的 v${srcs[0]} 是源点、出度 0 的 v${dst} 是汇点），弧=活动（权=耗时）；先 Kahn 拓扑保序（${topo.map(i => `v${i}`).join('→')}），ve 沿序取 max、vl 逆序取 min——单源单汇缺一不可`)]

  for (const u of topo) {
    for (const e of edges.filter(x => x.u === u)) {
      if (ve[u] + e.w > ve[e.v]) {
        const old = ve[e.v]
        ve[e.v] = ve[u] + e.w
        steps.push(gframe(nodesOf({ act: e.v, hl: [u] }), edgesOf(undefined, { u: e.u, v: e.v }), veAux(),
          `正推 ve（拓扑序取 max）：ve[${e.v}] = max(${old}, ve[${u}] + w(${u},${e.v}) = ${ve[u]} + ${e.w} = ${ve[e.v]}) = ${ve[e.v]}——事件最早也要等最慢的来路`))
      }
    }
  }

  // 逆推 vl
  for (let i = 0; i < n; i++) vl[i] = ve[dst]
  steps.push(gframe(nodesOf(), edgesOf(), veAux(),
    `vl 初始化为 ve[汇] = ${ve[dst]}（汇点不许拖延：vl[汇] = ve[汇]），开始逆拓扑序回推`))
  for (let k = topo.length; k > 0; k--) {
    const u = topo[k - 1]
    for (const e of edges.filter(x => x.u === u)) {
      if (vl[e.v] - e.w < vl[u]) {
        const old = vl[u]
        vl[u] = vl[e.v] - e.w
        steps.push(gframe(nodesOf({ act: u, hl: [e.v] }), edgesOf(undefined, { u: e.u, v: e.v }), veAux(),
          `逆推 vl（逆序取 min）：vl[${u}] = min(${old}, vl[${e.v}] − w(${u},${e.v}) = ${vl[e.v]} − ${e.w} = ${vl[u]}) = ${vl[u]}——最迟时刻由后继倒逼，减的是弧头`))
      }
    }
  }

  // 活动表：e = ve[u]、l = vl[v] − w
  const crit = new Set<string>()
  const acts = edges.map(e => ({ u: e.u, v: e.v, w: e.w, e: ve[e.u], l: vl[e.v] - e.w }))
  for (const a of acts) if (a.e === a.l) crit.add(`${a.u}-${a.v}`)
  steps.push(gframe(nodesOf({ hl: Array.from({ length: n }, (_, i) => i) }), edgesOf(crit), veAux(),
    `活动表逐弧算：e = ve[u]（最早开始）、l = vl[v] − w（最迟开始），e = l 即无余量的关键活动：${edges.map(e => `${e.u}→${e.v}: e=${ve[e.u]}, l=${vl[e.v] - e.w}${crit.has(`${e.u}-${e.v}`) ? ' ★关键' : ''}`).join('；')}`))

  steps.push(gframe(nodesOf({ hl: Array.from({ length: n }, (_, i) => i) }), edgesOf(crit), veAux(),
    `关键路径 = e == l 的活动串成的有向链，总工期 duration = ve[汇] = ${ve[dst]}；关键路径可能不唯一（等长路径并存），要缩短工期得一起缩——缩过头余量活动反成新瓶颈`))

  return steps
}
