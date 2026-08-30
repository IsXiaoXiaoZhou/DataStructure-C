// docs/.vitepress/theme/visualizer/steps/binSearch.ts
import type { Step } from '../types'

// 折半查找（对应 08_查找/02_折半查找 bin_search.c）
// 语义：要求 a 升序；low/high 夹逼，mid = low + (high-low)/2（防溢出写法）；
// 命中返回 mid，key < a[mid] 缩左边，否则缩右边，low > high 即失败
export interface BinSearchInput { a: number[]; key: number }

export function binSearchSteps(input: BinSearchInput): Step[] {
  const { a, key } = input
  const n = a.length
  const steps: Step[] = [{
    state: [...a], highlights: [], active: [0, n - 1],
    narration: `有序表 [${a.join(', ')}]，找 key = ${key}；low = 0，high = ${n - 1}——折半只对有序表成立`
  }]

  let low = 0
  let high = n - 1
  let found = -1
  let rounds = 0
  while (low <= high) {
    rounds++
    const mid = low + Math.floor((high - low) / 2)
    steps.push({
      state: [...a], highlights: [mid], active: [low, high],
      narration: `第 ${rounds} 轮：区间 [${low}, ${high}]，mid = ${mid}（${low} + (${high}−${low})/2），a[${mid}] = ${a[mid]}`
    })
    if (a[mid] === key) {
      found = mid
      steps.push({
        state: [...a], highlights: [mid], active: null,
        narration: `a[${mid}] = ${key} = key，命中！下标 ${mid}（位序 ${mid + 1}）`
      })
      break
    }
    if (key < a[mid]) {
      steps.push({
        state: [...a], highlights: [mid], active: [low, Math.max(low, mid - 1)],
        narration: `key ${key} < a[${mid}] = ${a[mid]}，目标只可能在左半 → high = ${mid} − 1 = ${mid - 1}`
      })
      high = mid - 1
    } else {
      steps.push({
        state: [...a], highlights: [mid], active: [Math.min(high, mid + 1), high],
        narration: `key ${key} > a[${mid}] = ${a[mid]}，目标只可能在右半 → low = ${mid} + 1 = ${mid + 1}`
      })
      low = mid + 1
    }
  }

  if (found >= 0) {
    steps.push({
      state: [...a], highlights: [found], active: null,
      narration: `折半查找成功：a[${found}] = ${key} 命中，共 ${rounds} 轮比较，每轮区间减半，O(log ${n})——同样的表顺序查找平均要比 (n+1)/2 ≈ ${(n + 1) / 2} 次`
    })
  } else {
    steps.push({
      state: [...a], highlights: [], active: null,
      narration: `low = ${low} > high = ${high}，区间已空 → 未找到 key = ${key}；${rounds} 轮就把范围缩没，失败也只花 O(log n)`
    })
  }
  return steps
}
