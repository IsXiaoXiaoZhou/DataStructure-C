// docs/.vitepress/theme/visualizer/steps/heapStringGrow.ts
import type { Step } from '../types'

// 堆分配串动态扩容（对应 05_串/02_堆分配串 heap_string.c 的 ensure_capacity + hs_assign）
// 源码语义：ensure_capacity 走 realloc(need+1)——可能原地扩展也可能另觅新块；
// 失败时原块仍有效、返回 DS_ERROR。动画按"另觅新块再整块搬运后 free 旧块"的
// 保守路径画，新容量按倍增教学口径给 2×旧容量（输入 5~8 字符时 need ≤ 8 恰好装下）。
export interface HeapGrowInput { src: string }
const OLD_CAP = 4

export function heapStringGrowSteps(input: HeapGrowInput): Step[] {
  const src = input.src
  const n = src.length
  const old = src.slice(0, OLD_CAP).split('')
  const fresh: (string | number)[] = []
  const newCap = OLD_CAP * 2
  const frame = (rows: {
    label: string; chars: (string | number)[]; capacity?: number
    charHighlight?: number[]; charActive?: number[]; strike?: number[]; pointers?: { name: string; index: number }[]
  }[], narration: string): Step =>
    ({ state: { rows: rows.map(r => ({ ...r, chars: [...r.chars], charHighlight: [...(r.charHighlight ?? [])], charActive: [...(r.charActive ?? [])], strike: [...(r.strike ?? [])], pointers: r.pointers?.map(p => ({ ...p })) })) }, highlights: [], active: null, narration })

  const steps: Step[] = [frame([
    { label: `旧块 s->ch（容量 ${OLD_CAP}）`, chars: old, capacity: OLD_CAP, pointers: [{ name: 'ch', index: OLD_CAP - 1 }] }
  ], `容量满：length = ${OLD_CAP} = capacity，现在要装入 ${n} 个字符（"源串"输入）——堆分配串与定长串分道扬镳的时刻：不报 DS_OVERFLOW，而是找堆要一块更大的`)]

  steps.push(frame([
    { label: `旧块 s->ch（容量 ${OLD_CAP}）`, chars: old, capacity: OLD_CAP },
    { label: `新块 ch'（容量 ${newCap} = 2×${OLD_CAP}，另有 1 格给 '\\0'）`, chars: [], capacity: newCap + 1 }
  ], `ensure_capacity(${n})：realloc 另觅新块——本动画按倍增策略给 2×${OLD_CAP} = ${newCap} 格（源码按需 realloc(need+1)；realloc 失败返回 DS_ERROR 且原块不动，无泄漏）`))

  for (let i = 0; i < n; i++) {
    fresh.push(src[i])
    const rows = [
      { label: `旧块 s->ch（容量 ${OLD_CAP}）`, chars: old, capacity: OLD_CAP, charActive: i < OLD_CAP ? [i] : [] },
      { label: `新块 ch'（容量 ${newCap}）length=${i + 1}`, chars: [...fresh], capacity: newCap, charHighlight: Array.from({ length: i }, (_, k) => k), charActive: [i], pointers: [{ name: 'i', index: i }] }
    ]
    const src2 = i < OLD_CAP ? '旧块' : `源串第 ${i + 1} 位`
    steps.push(frame(rows, `搬运 ${i + 1}/${n}：ch'[${i}] = ${src2}['${src[i]}']（逐字符拷入，长度域随之递增）`))
  }

  fresh.push('\\0')
  steps.push(frame([
    { label: `旧块 s->ch（容量 ${OLD_CAP}）`, chars: old, capacity: OLD_CAP, strike: Array.from({ length: OLD_CAP }, (_, k) => k) },
    { label: `新块 s->ch（容量 ${newCap} + 1 格 '\\0'）length=${n}`, chars: [...fresh], capacity: newCap + 1, pointers: [{ name: 'ch', index: n }] }
  ], `ch[${n}] = '\\0' 尾置终止符（支持只读 C 字符串访问），free(旧块)——灰色删除线即已释放；指针改指新块，capacity ${OLD_CAP} → ${newCap}`))

  steps.push(frame([
    { label: `旧块（已 free）`, chars: old, capacity: OLD_CAP, strike: Array.from({ length: OLD_CAP }, (_, k) => k) },
    { label: `新块 s->ch（容量 ${newCap} + 1 格 '\\0'）length=${n}`, chars: [...fresh], capacity: newCap + 1, charHighlight: Array.from({ length: n }, (_, k) => k), pointers: [{ name: 'ch', index: n }] }
  ], `扩容完成：${n} 个字符住进 ${newCap} 格新块。hs_clear 只做 length=0 保留缓冲复用，真正 free 在 hs_destroy——堆串的代价就是这对 malloc/free 必须成对`))

  return steps
}
