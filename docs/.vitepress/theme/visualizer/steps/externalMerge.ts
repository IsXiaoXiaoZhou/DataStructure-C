// docs/.vitepress/theme/visualizer/steps/externalMerge.ts
import type { Step } from '../types'

// 外部 k 路归并（对应 09_排序/10_外部排序/04_外部多路归并 external_merge.c）
// 源码语义：k 个已升序的输入文件，内嵌败者树选全局最小写输出；某文件读尽把其
// 段值置 INT_MAX（永不再胜出），胜者也是 INT_MAX 即全部 EOF、归并完成。
// 段数补位成不小于它的 2 次幂 size，tree[0] 存胜者、内部结点存败者。
// 动画以"逐轮比较各段队首"呈现等价的选择过程（k ≤ 4 时即败者树每轮的选择结果）。
export interface ExternalMergeInput { segs: number[][] }
const EOF = 'MAX'

export function externalMergeSteps(input: ExternalMergeInput): Step[] {
  const segs = input.segs.map(s => [...s])
  const k = segs.length
  const cursors: number[] = Array.from({ length: k }, () => 0)
  const eof: boolean[] = Array.from({ length: k }, () => false)
  const out: number[] = []

  const curVal = (i: number) => (eof[i] ? null : segs[i][cursors[i]])
  const rows = () => [
    ...segs.map((seg, i) => ({
      label: `输入文件 f${i}（升序段，剩 ${seg.length - cursors[i]} 条）`,
      chars: seg as (string | number)[],
      charHighlight: cursors[i] > 0 && !eof[i] ? Array.from({ length: cursors[i] }, (_, x) => x) : [],
      charActive: !eof[i] ? [cursors[i]] : [],
      strike: eof[i] ? seg.map((_, x) => x) : [],
      pointers: [{ name: `f${i}`, index: Math.min(cursors[i], seg.length - 1) }]
    })),
    {
      label: `输出文件 out（已写 ${out.length} 条）`,
      chars: out as (string | number)[],
      charHighlight: out.length ? out.map((_, x) => x) : [],
      pointers: [{ name: 'out', index: Math.max(0, out.length - 1) }]
    }
  ]
  const frame = (narration: string, note?: string): Step =>
    ({ state: { rows: rows().map(r => ({ ...r, charHighlight: [...(r.charHighlight ?? [])], charActive: [...(r.charActive ?? [])], strike: [...(r.strike ?? [])], pointers: r.pointers.map(p => ({ ...p })) })), ...(note ? { note } : {}) }, highlights: [], active: null, narration })

  const size = Math.max(1, 2 ** Math.ceil(Math.log2(Math.max(2, k)))) // 不小于 k 的最小 2 次幂

  const steps: Step[] = [frame(
    `外部 k 路归并开跑：k = ${k} 个升序段各读出队首（f 指针即游标）；内嵌败者树补位成 size = ${size}（≥k 的最小 2 次幂，补位段值 INT_MAX 永败），tree[0] 存胜者、内部结点存败者——动画用"逐轮比较队首"呈现同一选择结果，O(n log k)`,
    `败者树口径：叶子在 [size, 2size)、内部结点 tree[1..size) 存"输掉的段号"、tree[0] 存整体胜者`)]

  for (;;) {
    const alive = Array.from({ length: k }, (_, i) => i).filter(i => !eof[i])
    if (!alive.length) break
    const vals = alive.map(i => `${curVal(i)}(f${i})`)
    let win = alive[0]
    for (const i of alive) if (curVal(i)! < curVal(win)!) win = i   // 平手取段号小者（seg[wl] <= seg[wr] 取左）
    steps.push(frame(
      `比较各段队首：${vals.join(', ')} → 全局最小来自段 f${win}（值 ${curVal(win)}）——败者树一轮 adjust 就得到这个胜者${eof.some(x => x) ? '（MAX 段必败不参选）' : ''}`))
    out.push(curVal(win)!)
    steps.push(frame(
      `写入 out：fprintf(out, "%d") = ${out[out.length - 1]}（来自段 f${win}）→ 游标右移读下一条`,
      `已写 ${out.length} 条`))
    cursors[win]++
    if (cursors[win] >= segs[win].length) {
      eof[win] = true
      steps.push(frame(
        `f${win} 读尽：seg[${win}] = INT_MAX（EOF 语义）——该段从此每轮必败、永不再胜出，调整败者树后胜者易主`))
    }
  }

  steps.push(frame(
    `归并完成：胜者也是 INT_MAX → 全部 EOF，共写入 ${out.length} 条记录——全局升序 [${out.join(', ')}]；内存开销只 O(k)（败者树 + 段值缓冲），这就是外部排序"大文件小内存"的核心`))

  return steps
}
