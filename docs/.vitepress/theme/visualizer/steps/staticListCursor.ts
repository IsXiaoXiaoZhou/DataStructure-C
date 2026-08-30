// docs/.vitepress/theme/visualizer/steps/staticListCursor.ts
import type { Step } from '../types'

// 静态链表游标插入（对应 01_线性表/06_静态链表 static_linked_list.c）
// 语义：space[0] 备用链头，space[MAX-1] 数据链头结点，cur==0 等价 NULL；
// 插入 = 从备用链摘一个空闲分量，沿 cur 找到第 pos-1 个结点后
// "先挂后继、再接前驱"。动画用 MAX=10 的迷你布局：下标 0 备用链头、
// 下标 9 数据链头结点、数据分量从下标 1 起；state 为各分量的 cur 值
export interface StaticListCursorInput { list: number[]; pos: number; value: number }

const MAX = 10

export function staticListCursorSteps(input: StaticListCursorInput): Step[] {
  const { list, pos, value } = input
  const n = list.length
  const cur: number[] = new Array(MAX).fill(0)
  for (let i = 1; i < n; i++) cur[i] = i + 1     // 数据链 1→2→…→n
  for (let i = n + 1; i < MAX - 2; i++) cur[i] = i + 1 // 备用链 n+1→…→8（下标 9 是数据链头结点，永不入备用链）
  cur[MAX - 2] = 0                               // 备用链收尾（源码 list_init：space[MAX_SIZE-2].cur = 0）
  cur[0] = n + 1 <= MAX - 2 ? n + 1 : 0          // 备用链头指向首个空闲分量
  cur[MAX - 1] = n >= 1 ? 1 : 0                  // 头结点指向首个数据分量

  const steps: Step[] = [{
    state: [...cur], highlights: [], active: null,
    narration: `初始：数据链 头结点9 → 1 → … → ${n} → 0（cur=0 即 NULL），备用链 0 → ${n + 1} → … → 8 → 0；下标即数组的"地址"，cur 域就是指针`
  }]

  // 沿游标定位第 pos-1 个结点（从头结点走 pos-1 步）
  let prev = MAX - 1
  for (let s = 1; s <= pos - 1; s++) {
    prev = cur[prev]
    steps.push({
      state: [...cur], highlights: [], active: [prev],
      narration: `沿 cur 走第 ${s}/${pos - 1} 步到分量 ${prev}（存 ${list[s - 1]}）——静态链表定位第 ${pos - 1} 个结点只能顺链走，O(n)`
    })
  }

  // 从备用链头摘空闲分量（模拟 malloc）
  const newIndex = cur[0]
  const nextFree = cur[newIndex]
  cur[0] = nextFree
  steps.push({
    state: [...cur], highlights: [], active: [0],
    narration: `从备用链头摘下空闲分量 ${newIndex}（模拟 malloc）：space[0].cur 由 ${newIndex} 改指 ${nextFree}——分配/释放就是两条链的摘除与归还，O(1)`
  })

  // 先挂后继
  const succ = cur[prev]
  cur[newIndex] = succ
  steps.push({
    state: [...cur], highlights: [], active: [newIndex],
    narration: `新分量 ${newIndex} 存 ${value}，先挂后继：space[${newIndex}].cur = ${succ}（原 ${prev} 号的后继，顺序不能反）`
  })

  // 再接前驱
  cur[prev] = newIndex
  steps.push({
    state: [...cur], highlights: [], active: [prev, newIndex],
    narration: `前驱再挂新分量：space[${prev}].cur = ${newIndex}——与单链表"先挂后继、再接前驱"同构，游标改动本身 O(1)`
  })

  // 走一遍最终链
  const chain: number[] = []
  let p = cur[MAX - 1]
  while (p !== 0 && chain.length < MAX) { chain.push(p); p = cur[p] }
  steps.push({
    state: [...cur], highlights: chain, active: null,
    narration: `插入完成：沿头结点走链 9→${chain.join('→')}→0，位序 ${pos} 处即 ${value}，length ${n}→${n + 1}；下标顺序无关逻辑次序，链才说了算`
  })
  return steps
}
