// docs/.vitepress/theme/visualizer/steps/spDijkstra.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// Dijkstra 最短路径（对应 07_图/06_最短路径 shortest_path.c 的 sp_dijkstra，有向网）
// 源码语义：dist 初值 INF、dist[src] = 0；每轮取集外 dist 最小点（首个并列最小者）
// 定入 done，再对其全部出弧松弛（!done[v] && dist[u]+w < dist[v] 才更新并记 prev）；
// 剩余点均不可达则 break。入口有全矩阵负权检查（贪心前提）。
export interface DijkstraInput extends GraphBuildInput { src: number }
const INF = '∞'

export function spDijkstraSteps(input: DijkstraInput): Step[] {
  const { edges, n, src } = input
  const coords = ringCoords(n)
  const w: Record<string, number> = {}
  for (const e of edges) w[`${e.u}-${e.v}`] = e.w ?? 1

  const dist: (number | null)[] = Array.from({ length: n }, () => null) // null = INF
  const prev: number[] = Array.from({ length: n }, () => -1)
  const done: boolean[] = Array.from({ length: n }, () => false)
  dist[src] = 0

  const dTxt = (v: number) => (dist[v] === null ? INF : String(dist[v]))
  const nodesView = (active?: number) => coords.map((c, i) => ({
    id: i, label: `v${i}`, x: c.x, y: c.y,
    highlight: done[i], active: active === i
  }))
  const edgesView = (active?: { u: number; v: number }) => edges.map(e => ({
    from: e.u, to: e.v, weight: e.w ?? 1, directed: true,
    active: !!active && active.u === e.u && active.v === e.v
  }))
  const aux = () => [
    { text: `dist:  [${Array.from({ length: n }, (_, i) => dTxt(i)).join(', ')}]   prev:  [${prev.map(p => (p === -1 ? '−' : String(p))).join(', ')}]` },
    { text: `已确定 S: {${done.map((d, i) => (d ? i : null)).filter(x => x !== null).join(',')}}   源点: v${src}   dist[v]=∞ 表示尚未触及` }
  ]

  const steps: Step[] = [gframe(nodesView(), edgesView(), aux(),
    `Dijkstra（有向网，spg_add_arc 只写 w[u][v] 单侧）：dist[src] = 0、其余 ∞；每轮"取集外 dist 最小点定入 S，再松弛它的出弧"——贪心成立的前提是无负权弧（入口有全矩阵检查）`)]

  for (let k = 0; k < n; k++) {
    // 取集外 dist 最小点（首个并列最小者）
    let best = -1, bestD = Infinity
    for (let v = 0; v < n; v++) {
      if (!done[v] && dist[v] !== null && dist[v]! < bestD) { bestD = dist[v]!; best = v }
    }
    if (best === -1) {
      steps.push(gframe(nodesView(), edgesView(), aux(),
        `剩余点 dist 全为 ∞（源点到不了它们）：提前 break——这些点的最短路径不存在，记 ∞ 即可`))
      break
    }
    done[best] = true
    steps.push(gframe(nodesView(best), edgesView(), aux(),
      `第 ${k + 1} 轮：集外 dist 最小者是 v${best}（dist = ${bestD}${k === 0 ? '，源点自身' : ''}）→ 定入 S，其 dist 不再改变——这就是"首个并列最小者"的贪心选择`))
    // 出边松弛
    for (const e of edges) {
      if (e.u !== best) continue
      const v = e.v
      if (done[v]) continue
      const wuv = e.w ?? 1
      if (dist[best] !== null && (dist[v] === null || dist[best]! + wuv < dist[v]!)) {
        const old = dTxt(v)
        dist[v] = dist[best]! + wuv
        prev[v] = best
        steps.push(gframe(nodesView(), edgesView({ u: best, v }), aux(),
          `松弛弧 ${best}→${v}（权 ${wuv}）：dist[${v}] = ${old} > dist[${best}] + w = ${dist[best]} + ${wuv} = ${dist[v]} → 更新为 ${dist[v]}，prev[${v}] = ${best}（最短路经 v${best} 中转）`))
      }
    }
  }

  steps.push(gframe(nodesView(), edgesView(), aux(),
    `Dijkstra 完成：dist = [${Array.from({ length: n }, (_, i) => dTxt(i)).join(', ')}]；沿 prev 回溯可重建路径（如 v${src} 到某点的最短路倒着读 prev 再反转）——复杂度 O(n²)，负权弧请改用 Bellman-Ford`))

  return steps
}
