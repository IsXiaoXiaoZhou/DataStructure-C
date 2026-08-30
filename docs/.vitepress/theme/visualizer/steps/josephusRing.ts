// docs/.vitepress/theme/visualizer/steps/josephusRing.ts
import type { Step } from '../types'
import type { LnNode, ListNodeState } from './states'

// 约瑟夫环报数出列（对应 01_线性表/05_循环链表 main.c josephus_solve）
// 语义：n 人编号 1..n 围成循环链表（尾结点回指首元），从第 start 位起报数、
//       报到 m 者出列，前驱跨接摘除，其继任者顶替报数起点，直至剩最后一人。
//       出圈位置公式（源码注释）：out_pos = (cur + m - 2) % 剩余人数 + 1
//       start=1 时与 main.c 的 josephus_solve(n, m) 完全一致（n=8, m=3 → 3 6 1 5 2 8 4 7）
export interface JosephusRingInput { n: number; start: number; m: number }

export function josephusRingSteps(input: JosephusRingInput): Step[] {
  const { n, start, m } = input
  const steps: Step[] = []

  /** 由当前存活名单构建环形结点数组（尾结点 ring:true 回指首元） */
  const build = (people: number[], activeIds: number[], doneIds: number[], labels: Record<number, string> = {}): LnNode[] => {
    const lastIdx = people.length - 1
    return people.map((p, i) => ({
      id: p, value: p,
      next: i + 1 < people.length ? people[i + 1] : people[0],
      ring: i === lastIdx,
      highlight: activeIds.includes(p) ? 'active' : (doneIds.includes(p) ? 'done' : 'none'),
      label: labels[p]
    }))
  }
  const pushNodes = (nodes: LnNode[], headId: number | null, narration: string) => {
    steps.push({ state: { nodes, headId, headLabel: 'rear→next' } as ListNodeState, highlights: [], active: null, narration })
  }
  const push = (people: number[], narration: string, activeIds: number[] = [], doneIds: number[] = [], labels: Record<number, string> = {}) => {
    pushNodes(build(people, activeIds, doneIds, labels), people[0] ?? null, narration)
  }

  let people = Array.from({ length: n }, (_, i) => i + 1)
  push(people, `约瑟夫环：${n} 人编号 1~${n} 围坐一圈（尾结点回指首元成环，rear->next 即首元），从第 ${start} 位起报数，报到 ${m} 者出列；出圈位置 = (cur + m − 2) % 剩余人数 + 1`)

  let cur = start
  let round = 0
  while (people.length > 1) {
    round++
    const remain = people.length
    const counted: number[] = []
    // 报数逐节点高亮：第 k 报的人在第 (cur + k - 2) % remain + 1 位
    for (let k = 1; k <= m; k++) {
      const p = ((cur + k - 2) % remain) + 1
      const person = people[p - 1]
      counted.push(person)
      push(
        people,
        `第 ${round} 轮报数 ${k}/${m}：${person} 号报 ${k}${k < m ? '，继续往下数' : '——报到 m，出列'}`,
        [person], counted.slice(0, -1)
      )
    }
    const p = ((cur + m - 2) % remain) + 1
    const out = people[p - 1]
    const prevPos = ((p - 2 + remain) % remain)
    const prevPerson = people[prevPos]
    // 前驱跨接帧（目标尚在数组中，前驱的 next 已越过它）
    const bridged = build(people, [out], [], { [out]: '出列', [prevPerson]: '前驱' })
    const bi = bridged.findIndex(x => x.id === prevPerson)
    bridged[bi] = { ...bridged[bi], next: people[p % remain] }
    pushNodes(bridged, people[0] ?? null, `第 ${round} 轮出列的是 ${out} 号（报到 ${m}）：前驱 ${prevPerson} 号的 next 越过 ${out} 号直指其后继——摘链 O(1)，这是循环链表的主场`)
    // 摘除
    people = people.filter(x => x !== out)
    push(
      people,
      `${out} 号出列，剩 ${people.length} 人；其下一位（${people[((p - 1) % people.length)] ?? people[0]} 号）顶替报数起点，从 1 重新报数`,
      [], [], {}
    )
    cur = p
  }

  push(people, `只剩 ${people[0]} 号一人，${round} 轮出列结束`, [people[0]])
  return steps
}
