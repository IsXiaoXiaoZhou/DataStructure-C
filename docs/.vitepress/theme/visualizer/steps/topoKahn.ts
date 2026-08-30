// docs/.vitepress/theme/visualizer/steps/topoKahn.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// 拓扑排序 Kahn（对应 07_图/07_拓扑排序 topological_sort.c 的 topo_sort_kahn，AOV 网）
// 源码语义：入度统计 → 每轮取"编号最小的 0 入度未输出点"输出并削减其后继入度；
// 取不到 0 入度点即存在有向环（DS_ERROR"存在有向环，无法完成拓扑排序"）。
// 动画按队列口径呈现（编号序入队 = 源码"取编号最小"），aux 展示入度数组与队列。
export function topoKahnSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  const indeg: number[] = Array.from({ length: n }, () => 0)
  for (const e of edges) indeg[e.v]++

  const emitted: boolean[] = Array.from({ length: n }, () => false)
  const order: number[] = []
  const queue: number[] = Array.from({ length: n }, (_, i) => i).filter(i => indeg[i] === 0)

  const nodesView = (active?: number) => coords.map((c, i) => ({
    id: i, label: `v${i}`, x: c.x, y: c.y,
    highlight: emitted[i], active: active === i
  }))
  const edgesView = (active?: { u: number; v: number }) => edges.map(e => ({
    from: e.u, to: e.v, directed: true,
    active: !!active && active.u === e.u && active.v === e.v,
    label: emitted[e.u] && emitted[e.v] ? '已削' : undefined
  }))
  const aux = () => [
    { text: `indegree: [${indeg.join(', ')}]   队列: {${queue.join(',')}}   已输出: ${order.map(i => `v${i}`).join(' → ') || '∅'}` }
  ]

  const steps: Step[] = [gframe(nodesView(), edgesView(), aux(),
    `AOV 网拓扑排序（Kahn 入度削减法）：入度 0 的顶点无前驱、可立即输出——初始队列 {${queue.join(',')}}；本实现每轮取"编号最小的 0 入度点"，等价于按编号入队的 FIFO`)]

  for (let k = 0; k < n; k++) {
    const pick = queue.shift()
    if (pick === undefined) {
      steps.push(gframe(nodesView(), edgesView(), aux(),
        `队列空但已输出 ${order.length} < n = ${n}：剩余顶点互相卡住、入度都 ≥ 1——它们构成有向环，拓扑排序失败（DS_ERROR"存在有向环"）；环上每点入度 ≥ 1 正是判环依据`))
      return steps
    }
    order.push(pick)
    emitted[pick] = true
    steps.push(gframe(nodesView(pick), edgesView(), aux(),
      `出队输出 v${pick}（第 ${order.length} 个）——无前驱的任务先开工`))
    for (const e of edges) {
      if (e.u === pick) {
        indeg[e.v]--
        if (indeg[e.v] === 0 && !emitted[e.v] && !queue.includes(e.v)) queue.push(e.v)
        steps.push(gframe(nodesView(), edgesView({ u: e.u, v: e.v }), aux(),
          `削减后继：弧 ${pick}→${e.v} 撤离，indegree[${e.v}] = ${indeg[e.v] + 1} → ${indeg[e.v]}${indeg[e.v] === 0 ? `，归零 → 入队` : `，还没到 0、继续等前驱`}`))
      }
    }
  }

  steps.push(gframe(nodesView(), edgesView(), aux(),
    `拓扑序完成：${order.map(i => `v${i}`).join(' → ')}（共 ${order.length} = n 个全输出，无环）；同一 DAG 拓扑序可能不唯一，topo_verify 按弧方向逐条复核`))

  return steps
}
