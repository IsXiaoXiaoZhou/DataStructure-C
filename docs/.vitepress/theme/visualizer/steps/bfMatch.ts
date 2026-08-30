// docs/.vitepress/theme/visualizer/steps/bfMatch.ts
import type { Step } from '../types'

// 朴素模式匹配 BF（对应 05_串/04_朴素模式匹配 brute_force_match.c 的 match_core）
// 源码框架：start 对齐起点（0-based）+ 模式内偏移 i，失配 start+1、i 归零——
// 等价于教材 1-based 记法的"i 回溯到 i-j+2、j 归 1"。动画按教材 i/j 双指针画，
// narration 同时给源码口径（start 对齐起点前移一位）。
export interface BfMatchInput { s: string; t: string }

export function bfMatchSteps(input: BfMatchInput): Step[] {
  const s = input.s
  const t = input.t
  const n = s.length, m = t.length
  let cmp = 0
  let start = 0

  const rowS = (highlight: number[], active: number[], strike: number[], iAt: number) => ({
    label: `主串 S（${n} 个字符）`, chars: s.split(''), charHighlight: [...highlight], charActive: [...active], strike: [...strike],
    pointers: [{ name: 'i', index: iAt }]
  })
  const rowT = (highlight: number[], active: number[], jAt: number) => ({
    label: `模式串 T（${m} 个字符）`, chars: t.split(''), charHighlight: [...highlight], charActive: [...active],
    pointers: [{ name: 'j', index: jAt }]
  })
  const frame = (rowSArg: ReturnType<typeof rowS>, rowTArg: ReturnType<typeof rowT>, narration: string): Step =>
    ({ state: { rows: [rowSArg, rowTArg] }, highlights: [], active: null, narration })

  const steps: Step[] = [frame(
    rowS([], [], [], 0), rowT([], [], 0),
    `BF 匹配开始：主串长 n=${n}、模式串长 m=${m}。策略老实粗暴——从每个对齐起点逐一比较，失配就把主串指针回退一位重来，O(n×m)`)]

  let done = false
  for (start = 0; start + m <= n; start++) {
    for (let j = 0; j < m; j++) {
      cmp++
      if (s[start + j] === t[j]) {
        const matched = Array.from({ length: j + 1 }, (_, k) => start + k)
        steps.push(frame(
          rowS(matched, [start + j], [], start + j),
          rowT(Array.from({ length: j + 1 }, (_, k) => k), [j], j),
          `第 ${cmp} 次比较：S[${start + j + 1}]=${s[start + j]} == T[${j + 1}]=${t[j]} ✓（绿色为本次对齐已匹配段，j 继续右移）`))
      } else {
        const backSpan = Array.from({ length: j }, (_, k) => start + k)
        const nextStart = start + 1
        steps.push(frame(
          rowS([], [start + j], backSpan, nextStart),
          rowT([], [j], 0),
          `第 ${cmp} 次比较：S[${start + j + 1}]=${s[start + j]} ≠ T[${j + 1}]=${t[j]} ✗ 失配——灰色段（${j} 个已比字符）作废回退，教材口径 i 回到 i-j+2 = ${start + 2}、j 归 1；源码口径即对齐起点 start: ${start} → ${nextStart}${j === 0 ? '（i 本就在对齐起点，只右移一格、无回退损失）' : '，主串指针白白退回'}`))
        break
      }
      if (j === m - 1) {
        const matched = Array.from({ length: m }, (_, k) => start + k)
        steps.push(frame(
          rowS(matched, [], [], start + m - 1),
          rowT(Array.from({ length: m }, (_, k) => k), [], m - 1),
          `匹配成功（DS_OK）：模式在主串第 ${start + 1} 位命中（1-based，*pos = start+1 = ${start + 1}），共比较 ${cmp} 次——n、m 都小 BF 够用，规模一大就是 KMP 的主场`))
        done = true
      }
    }
    if (done) break
  }

  if (!done) {
    steps.push(frame(
      rowS([], [], [], n - 1), rowT([], [], 0),
      `未找到匹配位置（DS_ERROR，*pos = 0）：全部 ${Math.max(0, n - m + 1)} 个对齐起点都试完，共比较 ${cmp} 次——最坏情形每个起点都比到最后一位，O(n×m) 就是这么来的`))
  }

  return steps
}
