// docs/.vitepress/theme/visualizer/steps/kmpMatch.ts
import type { Step } from '../types'

// KMP 模式匹配（对应 05_串/05_KMP算法 kmp_match.c 的 kmp_get_next + match_core）
// 源码口径：next 表 1-based、长度 m+1、next[1]=0（弃用 [0] 位）；匹配时 0-based 已匹配
// j 个 == 失配于教材位号 j+1，跳转目标 = next[j+1]，主串 i 永不回退。
// 动画三行：主串 / 模式串 / next 表（row 下标 k 即位号 k+1）。
export interface KmpMatchInput { s: string; t: string }

export function kmpMatchSteps(input: KmpMatchInput): Step[] {
  const s = input.s
  const t = input.t
  const n = s.length, m = t.length
  const next: number[] = new Array(m + 1).fill(0) // next[1..m] 有效
  let filledCount = 1                              // 已定值的 next 位数（next[1]=0 先行）
  let cmp = 0

  const nextRow = (highlight: number[] = [], active: number[] = [], filled = filledCount) => ({
    label: `next 表（1-based：next[1]=0，弃用 [0] 位）`,
    chars: Array.from({ length: m }, (_, k) => (k < filled ? next[k + 1] : '')) as (string | number)[],
    charHighlight: [...highlight], charActive: [...active]
  })
  const rowS = (highlight: number[], active: number[], strike: number[], iAt: number) => ({
    label: `主串 S（${n} 个字符）`, chars: s.split(''), charHighlight: [...highlight], charActive: [...active], strike: [...strike],
    pointers: [{ name: 'i', index: iAt }]
  })
  const rowT = (highlight: number[], active: number[], jAt: number) => ({
    label: `模式串 T（${m} 个字符）`, chars: t.split(''), charHighlight: [...highlight], charActive: [...active],
    pointers: [{ name: 'j', index: jAt }]
  })
  const frame = (rs: ReturnType<typeof rowS>, rt: ReturnType<typeof rowT>, nx: ReturnType<typeof nextRow>, narration: string): Step =>
    ({ state: { rows: [rs, rt, nx] }, highlights: [], active: null, narration })

  // ---------- 阶段一：get_next（模式串自匹配） ----------
  const steps: Step[] = [frame(
    rowS([], [], [], 0), rowT([], [], 0), nextRow(),
    `阶段一：kmp_get_next 先给模式串自算 next 表——本实现 next[1] = 0（首字符失配时 j 归 0、配合 i++），next[j] = t[1..j-1] 最长相等前后缀长度 +1（1-based 位号）`)]

  let i = 1, j = 0
  next[1] = 0
  while (i < m) {
    if (j === 0 || t[i - 1] === t[j - 1]) {
      const reason = j === 0
        ? `j 已回退到头（=0）`
        : `t[${i}]='${t[i - 1]}' 与 t[${j}]='${t[j - 1]}' 相等`
      i++; j++
      next[i] = j
      filledCount = i
      steps.push(frame(
        rowS([], [], [], 0),
        rowT(j === 0 ? [] : [j - 1, i - 1], [i - 1], i - 1),
        nextRow([i - 1], []),
        `next[${i}] = ${j}：${reason}，前后缀匹配长度 +1${j === 0 ? '（next[i] 记 1：从这里起没有更短的相等前后缀可借力）' : ''}`))
    } else {
      const oldJ = j
      j = next[j]
      steps.push(frame(
        rowS([], [], [], 0),
        rowT([], [i - 1], i - 1),
        nextRow([], [oldJ - 1]),
        `t[${i}]='${t[i - 1]}' ≠ t[${oldJ}]='${t[oldJ - 1]}'：沿 next 链回退 j = next[${oldJ}] = ${j}，对次长相等前后缀再试（这正是匹配阶段将要做的同款跳跃）`))
    }
  }

  const nextSummary = Array.from({ length: m }, (_, k) => next[k + 1]).join(', ')
  steps.push(frame(
    rowS([], [], [], 0), rowT([], [], 0), nextRow(Array.from({ length: m }, (_, k) => k)),
    `next 表就绪：next[1..${m}] = [${nextSummary}]。两个锚点自查——next[1]=0、next[2]=1；手算填错多半是差了这 1`))

  // ---------- 阶段二：match_core（i 不回退） ----------
  steps.push(frame(
    rowS([], [], [], 0), rowT([], [], 0), nextRow(Array.from({ length: m }, (_, k) => k)),
    `阶段二：kmp_index 匹配开始——主串指针 i 只进不退，失配时模式指针 j 沿 next 表跳，O(n+m) 对 BF 的 O(n×m)`))

  let si = 0, sj = 0
  let done = false
  while (si < n && sj < m) {
    cmp++
    if (s[si] === t[sj]) {
      steps.push(frame(
        rowS(Array.from({ length: sj + 1 }, (_, k) => si - sj + k), [si], [], si),
        rowT(Array.from({ length: sj + 1 }, (_, k) => k), [sj], sj),
        nextRow(Array.from({ length: m }, (_, k) => k)),
        `第 ${cmp} 次比较：S[${si + 1}]='${s[si]}' == T[${sj + 1}]='${t[sj]}' ✓，i、j 同步右移（已匹配 ${sj + 1} 位）`))
      si++; sj++
    } else if (sj === 0) {
      steps.push(frame(
        rowS([], [si], [], si + 1),
        rowT([], [0], 0),
        nextRow(Array.from({ length: m }, (_, k) => k)),
        `第 ${cmp} 次比较：S[${si + 1}]='${s[si]}' ≠ T[1]='${t[0]}' ✗ 模式第 1 字符就失配——主串右移一格（i++），j 保持 0，正是 next[1]=0 的约定`))
      si++
    } else {
      const j1 = next[sj + 1]
      steps.push(frame(
        rowS(Array.from({ length: sj }, (_, k) => si - sj + k), [si], [], si),
        rowT(Array.from({ length: sj }, (_, k) => k), [sj], sj),
        nextRow([], [sj]),
        `第 ${cmp} 次比较：S[${si + 1}]='${s[si]}' ≠ T[${sj + 1}]='${t[sj]}' ✗ 失配于模式第 ${sj + 1} 位——i 不回退！查 next[${sj + 1}] = ${j1}，j 跳到 ${j1 === 0 ? 0 : j1 - 1}${j1 === 0 ? '（整段重来：i 右移一格）' : `，前 ${j1 - 1} 个字符经前后缀关系保证已匹配、免比`}`))
      if (j1 === 0) { si++; sj = 0 } else { sj = j1 - 1 }
    }
    if (sj === m) {
      const pos = si - m + 1
      steps.push(frame(
        rowS(Array.from({ length: m }, (_, k) => pos - 1 + k), [], [], si - 1),
        rowT(Array.from({ length: m }, (_, k) => k), [], m - 1),
        nextRow(Array.from({ length: m }, (_, k) => k)),
        `匹配成功（DS_OK）：*pos = i-m+1 = ${pos}（1-based），共比较 ${cmp} 次——i 从头到尾没回退过一步，这就是与 BF 的本质对照`))
      done = true
    }
  }

  if (!done) {
    steps.push(frame(
      rowS([], [], [], n - 1), rowT([], [], 0), nextRow(Array.from({ length: m }, (_, k) => k)),
      `未找到匹配位置（DS_ERROR，*pos = 0）：i 扫完主串共比较 ${cmp} 次——虽然没命中，比较次数仍被 next 表压住，最坏也是 O(n)`))
  }

  return steps
}
