// docs/.vitepress/theme/visualizer/steps/unionFindOps.ts
// 并查集 union/find（对应 06_树/07_并查集/union_find.c）：
//   双亲表示数组 parent[x]，根的 parent 指向自身；size[] 仅根有效。
//   find 两趟式路径压缩（迭代）：第一趟沿链找根，第二趟沿途结点 parent 全部直挂根；
//   union 按大小合并：先 find 两根（顺带压缩），size[rx] < size[ry] 才交换（等大时
//   y 的根挂 x 的根），parent[ry] = rx、size[rx] += size[ry]、sets--。
// 输入 "u:1,2,u:3,4,f:2,u:1,3"（u:合并 f:查找）；元素 1..max（源码 0 起下标，演示用 1 起）。
// 画面为森林形态（虚拟总根汇总各代表元），本质就是双亲表示的树状视图。
import type { Step } from '../types'
import { tstep, mnode, vroot } from './treeKit'

export interface UfOp { op: 'u' | 'f'; a: number; b?: number }
export interface UnionFindInput { ops: UfOp[]; n: number }

export function unionFindOpsSteps(input: UnionFindInput): Step[] {
  const steps: Step[] = []
  const { ops, n } = input
  const parent: number[] = Array.from({ length: n + 1 }, (_, i) => i) // 1..n，根指向自身
  const size: number[] = Array(n + 1).fill(1)
  let sets = n

  /** 森林帧：虚拟总根 0 汇总当前所有根；hotEdge=(父,子) 高亮一条 parent 边 */
  const frame = (narration: string, note?: string, act: number[] = [], done: number[] = [], hotEdge?: { p: number; c: number }) => {
    const elems = Array.from({ length: n }, (_, k) => k + 1)
    const kidsOf = (x: number) => elems.filter(c => c !== x && parent[c] === x).sort((a, b) => a - b)
    const roots = elems.filter(x => parent[x] === x)
    const nodes = [
      vroot(0, roots, `森林（${sets} 个集合）`),
      ...elems.map(x => mnode(x, x, kidsOf(x), {
        nodeLabel: parent[x] === x ? `size=${size[x]}` : `p=${parent[x]}`,
        keyActive: act.includes(x) ? [0] : undefined,
        keyHighlight: done.includes(x) ? [0] : undefined,
        edgeActive: hotEdge && hotEdge.p === x && kidsOf(x).includes(hotEdge.c)
          ? [kidsOf(x).indexOf(hotEdge.c)]
          : undefined
      }))
    ]
    steps.push(tstep(nodes, 0, narration, note))
  }

  /** find 动画：第一趟路径帧 + 第二趟逐点改挂帧；返回根 */
  const animateFind = (x: number, tag: string): number => {
    // 第一趟：沿双亲链找根
    const path: number[] = []
    let r = x
    while (parent[r] !== r) { path.push(r); r = parent[r] }
    path.push(r)
    frame(`${tag} 第一趟：沿 parent 上行 ${path.join('→')}（parent[${r}]=${r} 即根），根 = ${r}`,
      'find 两趟式：先沿链找到根，再从 x 走一遍把沿途结点 parent 直指根——压成扇形，防深链', [x], [], undefined)
    // 第二趟：沿途直挂根（逐点改挂帧）
    let p = x
    while (parent[p] !== r) {
      const next = parent[p]
      parent[p] = r
      frame(`路径压缩：${p} 的 parent 直接改为根 ${r}（原来是 ${next}）——沿途结点全部直挂根，树被压成"扇形"`,
        undefined, [p], [r], { p: r, c: p })
      p = next
    }
    return r
  }

  // 0. 开场
  frame(`初始化：${n} 个元素各自成根（parent[i] = i，size 全 1），sets = ${n}`,
    '双亲表示数组：parent[x] 存双亲，根的 parent 指向自身；size[] 仅根有效，支持按大小合并', [], [], undefined)

  let merged = 0
  for (const op of ops) {
    if (op.op === 'u') {
      const x = op.a, y = op.b as number
      const rx = animateFind(x, `union(${x},${y}) 之 find(${x})`)
      const ry = animateFind(y, `union(${x},${y}) 之 find(${y})`)
      if (rx === ry) {
        frame(`find(${x}) 与 find(${y}) 同根 ${rx}——本就同集，无需合并（merged=0）`,
          undefined, [rx], [], undefined)
        continue
      }
      const swap = size[rx] < size[ry]
      frame(`按大小合并：size[${rx}]=${size[rx]} vs size[${ry}]=${size[ry]}——${swap ? `${rx} 树更小，交换（小树挂大树）` : size[rx] === size[ry] ? '等大时任挂（"小树挂大树"的等大特例）：源码仅当 size[rx] < size[ry] 才交换，故后者挂前者' : `${ry} 树更小（小树挂大树）`}`,
        'union 按大小合并保证树高 O(log n)；与路径压缩合用后单操作均摊 O(α(n))', [rx, ry], [], undefined)
      const big = swap ? ry : rx
      const small = swap ? rx : ry
      parent[small] = big
      size[big] += size[small]
      sets--
      merged++
      frame(`挂接：parent[${small}] = ${big}，size[${big}] = ${size[big]}，sets 减到 ${sets}（第 ${merged} 次合并）`,
        undefined, [big, small], [], { p: big, c: small })
    } else {
      const x = op.a
      const r = animateFind(x, `find(${x})`)
      frame(`find(${x}) = ${r}${parent[x] === r ? `（${x} 已直挂根，无需再压）` : ''}——uf_connected 判连通即比较两根是否相同`,
        undefined, [r], [x], undefined)
    }
  }

  const roots = Array.from({ length: n }, (_, k) => k + 1).filter(x => parent[x] === x)
  frame(`演示结束：剩 ${sets} 个集合，代表元 ${roots.join('、')}——向上找爹 O(α(n))，并查集的正确姿势`,
    `应用：等价类 / 连通性判定 / Kruskal 最小生成树判环（07_图 模块）`, roots, [], undefined)

  return steps
}
