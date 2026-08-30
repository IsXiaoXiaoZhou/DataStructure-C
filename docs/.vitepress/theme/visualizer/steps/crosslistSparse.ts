// docs/.vitepress/theme/visualizer/steps/crosslistSparse.ts
import type { Step } from '../types'
import { gframe } from './graphKit'

// 稀疏矩阵十字链表 set（对应 04_特殊矩阵压缩存储/05_稀疏矩阵-十字链表 cross_sparse_matrix.c
// 的 matrix_set）。源码语义：每个非零元一个结点同时挂入两条有序链——行链按列升序
// （right 串）、列链按行升序（down 串）；rhead/chead 两套头指针数组。
// 布局：结点画在 (列→x, 行→y) 的网格坐标上，right 边水平、down 边竖直（有向）。
export interface CrossListInput { cells: { i: number; j: number; v: number }[]; rows: number; cols: number }

interface Hot { kind: 'right' | 'down'; from: number; to: number } // from/to 为结点 id（i*cols+j）

export function crosslistSparseSteps(input: CrossListInput): Step[] {
  const { rows, cols } = input
  const nodes: { i: number; j: number; v: number }[] = []
  const idOf = (i: number, j: number) => i * cols + j

  const pos = (i: number, j: number) => ({ x: 14 + (72 * j) / Math.max(1, cols - 1), y: 14 + (72 * i) / Math.max(1, rows - 1) })
  const nodesOf = (act?: { i: number; j: number }) => nodes.map(nd => ({
    id: idOf(nd.i, nd.j), label: `${nd.v}`, x: +pos(nd.i, nd.j).x.toFixed(2), y: +pos(nd.i, nd.j).y.toFixed(2),
    active: !!act && act.i === nd.i && act.j === nd.j
  }))
  const edgesOf = (hot?: Hot) => {
    const out: { from: number; to: number; directed: boolean; label?: string; active?: boolean }[] = []
    for (let i = 0; i < rows; i++) {           // 行链 right：同行结点按列升序
      const rowNodes = nodes.filter(nd => nd.i === i).sort((a, b) => a.j - b.j)
      for (let x = 0; x + 1 < rowNodes.length; x++) {
        const a = rowNodes[x], b = rowNodes[x + 1]
        out.push({
          from: idOf(a.i, a.j), to: idOf(b.i, b.j), directed: true, label: 'right',
          active: !!hot && hot.kind === 'right' && hot.from === idOf(a.i, a.j) && hot.to === idOf(b.i, b.j)
        })
      }
    }
    for (let j = 0; j < cols; j++) {           // 列链 down：同列结点按行升序
      const colNodes = nodes.filter(nd => nd.j === j).sort((a, b) => a.i - b.i)
      for (let x = 0; x + 1 < colNodes.length; x++) {
        const a = colNodes[x], b = colNodes[x + 1]
        out.push({
          from: idOf(a.i, a.j), to: idOf(b.i, b.j), directed: true, label: 'down',
          active: !!hot && hot.kind === 'down' && hot.from === idOf(a.i, a.j) && hot.to === idOf(b.i, b.j)
        })
      }
    }
    return out
  }
  const aux = () => {
    const lines: { text: string }[] = []
    for (let i = 0; i < rows; i++) {
      const r = nodes.filter(nd => nd.i === i).sort((a, b) => a.j - b.j)
      lines.push({ text: `rhead[${i}] → ${r.length ? r.map(nd => `(${i},${nd.j})=${nd.v}`).join('─right→') + ' → NULL' : 'NULL'}` })
    }
    for (let j = 0; j < cols; j++) {
      const c = nodes.filter(nd => nd.j === j).sort((a, b) => a.i - b.i)
      lines.push({ text: `chead[${j}] → ${c.length ? c.map(nd => `(${nd.i},${j})=${nd.v}`).join('─down→') + ' → NULL' : 'NULL'}` })
    }
    return lines
  }
  const frame = (narration: string, act?: { i: number; j: number }, hot?: Hot): Step =>
    gframe(nodesOf(act), edgesOf(hot), aux(), narration)

  const steps: Step[] = [gframe([], [], aux(),
    `十字链表存稀疏矩阵：${rows}×${cols}，头指针数组 rhead[0..${rows - 1}]（行链首）与 chead[0..${cols - 1}]（列链首）全空——每个非零元一个结点，同时属一条行链与一条列链，"十字"由此得名（网格上水平=right、竖直=down）`)]

  input.cells.forEach((cell, idx) => {
    const { i, j, v } = cell
    const rowGap = nodes.filter(nd => nd.i === i && nd.j < j).length
    const colGap = nodes.filter(nd => nd.j === j && nd.i < i).length
    nodes.push(cell)
    steps.push(frame(
      `set(${i},${j}) = ${v}（第 ${idx + 1}/${input.cells.length} 个非零元）：行链按列升序找缝隙——插到第 ${rowGap + 1} 位（前面已有 ${rowGap} 个列号更小的结点），列链同理插第 ${colGap + 1} 位；插的是"缝隙"不是表尾，prev→cur 改接顺序写漏一步就断链`,
      { i, j }))
    steps.push(frame(
      `挂十字：新结点 right 接进行链、down 接进列链，两链同时有序——与三元组"建好不动"的本质差异：十字链表插删只改指针不搬数据，代价是每结点多两个指针 + 两套头数组`,
      { i, j }))
  })

  steps.push(gframe(nodesOf(), edgesOf(), aux(),
    `建表完成：${nodes.length} 个结点、每结点挂 2 条链（right + down）；手画高频——别漏画 rhead/chead 两套头数组，方向口诀"right 横串列号增大、down 竖串行号增大"；set/get 定位 O(该行非零元数 + 该列非零元数)`))

  return steps
}
