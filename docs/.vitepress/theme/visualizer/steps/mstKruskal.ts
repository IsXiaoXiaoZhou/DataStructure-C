// docs/.vitepress/theme/visualizer/steps/mstKruskal.ts
import type { Step } from '../types'
import { gframe, ringCoords, ukey, type GraphBuildInput } from './graphKit'

// Kruskal 最小生成树（对应 07_图/05_最小生成树 min_spanning_tree.c 的 mst_kruskal）
// 源码语义：收集上三角边按权升序排序（平手按 u、v 字典序），逐条尝试——
// 内置并查集 find（路径压缩）判两端是否同集，同集必成环舍弃；
// 否则按大小合并并把边收入树，选满 n-1 条即停；选不满即不连通。
export function mstKruskalSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)

  // 收集边（规范成 u<v），按 w 升序、平手 (u,v) 字典序——与 edge_cmp 一致
  const all = edges.map(e => ({ u: Math.min(e.u, e.v), v: Math.max(e.u, e.v), w: e.w ?? 1 }))
    .sort((a, b) => a.w - b.w || a.u - b.u || a.v - b.v)

  const parent: number[] = Array.from({ length: n }, (_, i) => i)
  const size: number[] = Array.from({ length: n }, () => 1)
  const find = (x: number): number => {
    let r = x
    while (parent[r] !== r) r = parent[r]
    while (parent[x] !== r) { const next = parent[x]; parent[x] = r; x = next } // 路径压缩
    return r
  }

  const treeEdges: { u: number; v: number; w: number }[] = []
  const nodesView = (active?: number) => coords.map((c, i) => ({
    id: i, label: `v${i}`, x: c.x, y: c.y,
    highlight: treeEdges.some(t => t.u === i || t.v === i),
    active: active === i
  }))
  const edgesView = (active?: { u: number; v: number }) => all.map(e => ({
    from: e.u, to: e.v, weight: e.w,
    highlight: treeEdges.some(t => ukey(t.u, t.v) === ukey(e.u, e.v)),
    active: !!active && ukey(active.u, active.v) === ukey(e.u, e.v)
  }))
  const aux = () => [
    { text: `parent: [${parent.join(', ')}]  （并查集，find 带路径压缩）` },
    { text: `已收边: ${treeEdges.map(t => `${t.u}-${t.v}(${t.w})`).join('、') || '无'}  ${treeEdges.length}/${n - 1}` }
  ]

  const steps: Step[] = [gframe(nodesView(), edgesView(), aux(),
    `Kruskal 开局：全部 ${all.length} 条边按权升序排好（平手按 u、v 字典序，与 edge_cmp 一致）——${all.map(e => `${e.u}-${e.v}:${e.w}`).join('、')}。并查集初始化 parent[i] = i，逐条尝试"能接就接、成环就扔"`)]

  let total = 0
  for (const e of all) {
    if (treeEdges.length >= n - 1) break
    const ru = find(e.u), rv = find(e.v)
    if (ru === rv) {
      steps.push(gframe(nodesView(), edgesView({ u: e.u, v: e.v }), aux(),
        `尝试 ${e.u}-${e.v}（权 ${e.w}）：find 两端同根（${ru}）——加这条边必成环，舍弃（不删边不重排，继续看下一条）`))
      continue
    }
    // 按大小合并（源码仅 size[ru] < size[rv] 才交换）
    let a = ru, b = rv
    if (size[a] < size[b]) { const t = a; a = b; b = t }
    parent[b] = a
    size[a] += size[b]
    treeEdges.push(e)
    total += e.w
    steps.push(gframe(nodesView(), edgesView({ u: e.u, v: e.v }), aux(),
      `尝试 ${e.u}-${e.v}（权 ${e.w}）：两端不同集（根 ${ru} / ${rv}）→ 按大小合并（小树挂大树：parent[${b}] = ${a}，与源码"仅 size[ru] < size[rv] 才交换"一致）→ 边入选，总权 ${total}，已收 ${treeEdges.length}/${n - 1}`))
    if (treeEdges.length === n - 1) {
      steps.push(gframe(nodesView(), edgesView(), aux(),
        `Kruskal 完成：凑满 n-1 = ${n - 1} 条边即停（剩余候选边不必再看），总权 = ${total}——与 Prim 同权不同形是正常的（等权边时树形态可能不同）`))
      return steps
    }
  }

  if (treeEdges.length < n - 1) {
    steps.push(gframe(nodesView(), edgesView(), aux(),
      `边试完仍只有 ${treeEdges.length} 条（< n-1 = ${n - 1}）：图不连通，无法生成 MST（源码返回 DS_ERROR）`))
  }

  return steps
}
