// docs/.vitepress/theme/visualizer/steps/topoKahn.ts
import type { Step } from '../types'
import { gframe, ringCoords, type GraphBuildInput } from './graphKit'

// 拓扑排序 Kahn（对应 07_图/07_拓扑排序 topological_sort.c 的 topo_sort_kahn，AOV 网）
// 源码语义：入度统计 → 每轮线性扫描取"编号最小的 0 入度未输出点"输出并削减其后继入度
// （源码不建队列，for i: 0..n-1 取第一个满足者）；扫不到 0 入度点即存在有向环
// （DS_ERROR"存在有向环，无法完成拓扑排序"）。
// 动画与源码同构：每轮重扫 indegree 取最小编号（非 FIFO 队列——后继按边序入队会打破
// 编号序），aux 展示入度数组与当前候选集；拓扑序不唯一，取最小编号只是钉死一种输出。
export function topoKahnSteps(input: GraphBuildInput): Step[] {
  const { edges, n } = input
  const coords = ringCoords(n)
  const indeg: number[] = Array.from({ length: n }, () => 0)
  for (const e of edges) indeg[e.v]++

  const emitted: boolean[] = Array.from({ length: n }, () => false)
  const order: number[] = []
  // 当前 0 入度且未输出的候选集（源码每轮重扫 indeg/emitted 数组，这里等价物化出来展示）
  const candidates = (): number[] =>
    Array.from({ length: n }, (_, i) => i).filter(i => !emitted[i] && indeg[i] === 0)

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
    { text: `indegree: [${indeg.join(', ')}]   0入度候选: {${candidates().join(',')}}   已输出: ${order.map(i => `v${i}`).join(' → ') || '∅'}` }
  ]

  const steps: Step[] = [gframe(nodesView(), edgesView(), aux(),
    `AOV 网拓扑排序（Kahn 入度削减法）：入度 0 的顶点无前驱、可立即输出——与源码 topo_sort_kahn 同构：不建队列，每轮重扫 indegree 数组、取编号最小的 0 入度点（初始候选 {${candidates().join(',')}}）；拓扑序不唯一，取最小编号只是钉死一种输出`)]

  for (let k = 0; k < n; k++) {
    const cand = candidates()
    if (cand.length === 0) {
      steps.push(gframe(nodesView(), edgesView(), aux(),
        `第 ${k + 1} 轮扫描：已输出 ${order.length} < n = ${n} 却扫不到 0 入度点——剩余顶点互相卡住、入度都 ≥ 1，构成有向环，拓扑排序失败（DS_ERROR"存在有向环"）；环上每点入度 ≥ 1 正是判环依据`))
      return steps
    }
    const pick = cand[0] // 编号最小（源码 for i = 0..n-1 取第一个 !emitted && indeg==0）
    order.push(pick)
    emitted[pick] = true
    steps.push(gframe(nodesView(pick), edgesView(), aux(),
      `第 ${order.length} 轮扫描：候选 {${cand.join(',')}} 中编号最小的是 v${pick} → 输出（第 ${order.length} 个）——无前驱的任务先开工`))
    for (const e of edges) {
      if (e.u === pick) {
        indeg[e.v]--
        steps.push(gframe(nodesView(), edgesView({ u: e.u, v: e.v }), aux(),
          `削减后继：弧 ${pick}→${e.v} 撤离，indegree[${e.v}] = ${indeg[e.v] + 1} → ${indeg[e.v]}${indeg[e.v] === 0 ? `，归零 → 进入下一轮候选` : `，还没到 0、继续等前驱`}`))
      }
    }
  }

  steps.push(gframe(nodesView(), edgesView(), aux(),
    `拓扑序完成：${order.map(i => `v${i}`).join(' → ')}（共 ${order.length} = n 个全输出，无环）；同一 DAG 拓扑序不唯一——每轮选别的 0 入度点也合法，topo_verify 按弧方向逐条复核`))

  return steps
}
