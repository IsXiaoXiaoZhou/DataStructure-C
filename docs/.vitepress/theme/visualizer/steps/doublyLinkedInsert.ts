// docs/.vitepress/theme/visualizer/steps/doublyLinkedInsert.ts
import type { Step } from '../types'
import type { LnNode, ListNodeState } from './states'

// 双链表按位插入（对应 01_线性表/04_双链表 doubly_linked_list.c list_insert_at）
// 语义：带头结点 + tail 不变量；定位第 pos-1 个结点后，四条指针按源码顺序逐条改写：
//   ① node->prev = prev        ② node->next = prev->next
//   ③ prev->next->prev = node（后继为 NULL 时改 tail = node）
//   ④ prev->next = node
export interface DoublyLinkedInsertInput { pos: number; value: number }

const INIT = [12, 25, 33, 47, 58]
const HEAD_ID = 0
const NEW_ID = 100
const dataId = (i: number) => i + 1

type Entry = { id: number; value: number | string }
const dataSeq = (): Entry[] => [{ id: HEAD_ID, value: 'head' }, ...INIT.map((v, i) => ({ id: dataId(i), value: v }))]

export function doublyLinkedInsertSteps(input: DoublyLinkedInsertInput): Step[] {
  const { pos, value } = input
  const n = INIT.length
  const steps: Step[] = []
  const push = (nodes: LnNode[], narration: string, active: number[] | null = null, highlights: number[] = [], note?: string) => {
    steps.push({ state: { nodes, headId: HEAD_ID, note } as ListNodeState, highlights, active, narration })
  }

  /** 双链 seq → 结点数组（prev 修正量：需改写 prev 的结点 id → 新前驱 id） */
  const chain = (
    seq: Entry[],
    hl: (i: number) => 'none' | 'active' | 'done',
    labels: Record<number, string> = {},
    prevFix: Record<number, number> = {},
    nextFix: Record<number, number | null> = {}
  ): LnNode[] =>
    seq.map((s, i) => ({
      id: s.id, value: s.value,
      next: (s.id in nextFix ? nextFix[s.id] : (i + 1 < seq.length ? seq[i + 1].id : null)),
      prev: (s.id in prevFix ? prevFix[s.id] : (s.id === HEAD_ID ? null : seq[i - 1].id)),
      head: s.id === HEAD_ID,
      highlight: hl(i),
      label: labels[s.id]
    }))

  // 初始帧
  push(
    chain(dataSeq(), () => 'none'),
    `初始：双链表 head ⇄ ${INIT.join(' ⇄ ')} → NULL（length=${n}，head 的 next 指首元、首元的 prev 回指 head）；在第 ${pos} 位插入 ${value}，同样先定位第 ${pos - 1} 个结点`
  )

  // 定位
  if (pos === 1) {
    push(
      chain(dataSeq(), i => (i === 0 ? 'active' : 'none'), { [HEAD_ID]: 'prev' }),
      `pos = 1：第 0 个结点就是头结点（首元的 prev 恒指 &head），prev 原地不动——头插与中部插同构，零特判`,
      [0]
    )
  } else {
    for (let k = 1; k <= pos - 1; k++) {
      const labels: Record<number, string> = { [HEAD_ID]: '起点' }
      for (let j = 1; j <= k; j++) labels[dataId(j - 1)] = j === k ? 'prev' : '已走'
      push(
        chain(dataSeq(), i => (i > 0 && i <= k ? (i === k ? 'active' : 'done') : 'none'), labels),
        `定位第 ${k}/${pos - 1} 步：prev 沿 next 走到第 ${k} 个结点（值 ${INIT[k - 1]}）${k < pos - 1 ? '' : '——四条指针改写即将开始'}`,
        [k]
      )
    }
  }

  // 新结点悬停
  const prevId = pos === 1 ? HEAD_ID : dataId(pos - 2)
  const succId = pos <= n ? dataId(pos - 1) : null
  const hover: LnNode = { id: NEW_ID, value, prev: null, next: null, highlight: 'none', label: '新结点' }
  push(
    [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none')), hover],
    `malloc 新结点（data=${value}, prev=next=NULL）：双向接链前它悬在链外；四条指针按"先挂自己两条腿，再让两边回手"的顺序逐条改写`,
    null, [], `list_insert_at(list, ${pos}, ${value})`
  )

  // ① node->prev = prev
  push(
    [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none'), { [prevId]: 'prev' }), { ...hover, prev: prevId, highlight: 'active' }],
    `① node->prev = prev：新结点左腿踩上${pos === 1 ? '头结点' : `前驱 ${INIT[pos - 2]}`}——此刻它仍悬在链外，只挂了一条腿`,
    [n + 1]
  )

  // ② node->next = prev->next
  push(
    [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none'), { ...(pos <= n ? { [dataId(pos - 1)]: '原后继' } : {}), ...(pos > 1 ? { [dataId(pos - 2)]: 'prev' } : {}) }),
      { ...hover, prev: prevId, next: succId, highlight: 'active' }],
    `② node->next = prev->next：右腿接上原后继 ${succId != null ? INIT[pos - 1] : 'NULL'}——两条腿挂完，原链依然完好无损`,
    [n + 1]
  )

  // ③ prev->next->prev = node（后继为 NULL 时 tail = node）
  if (succId != null) {
    push(
      [...chain(dataSeq(), i => (i === pos ? 'active' : (i > 0 && i < pos ? 'done' : 'none')), { [succId]: '后继' }, { [succId]: NEW_ID }),
        { ...hover, prev: prevId, next: succId, highlight: 'active' }],
      `③ prev->next->prev = node：后继 ${INIT[pos - 1]} 的 prev 回指新结点（本帧已画出回指箭头）——漏了这条回手，从后往前遍历到这就断了`,
      [pos, n + 1]
    )
  } else {
    push(
      [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none'), { [prevId]: 'prev（原尾结点）' }),
        { ...hover, prev: prevId, next: succId, highlight: 'active', tail: true }],
      `③ 后继为 NULL（插在尾后）：tail 改指新结点——tail 不变量"非空时恒指最后一个数据结点"在此同步维护`,
      [n + 1]
    )
  }

  // ④ prev->next = node（新结点归位）
  const seq2: Entry[] = dataSeq()
  seq2.splice(pos, 0, { id: NEW_ID, value })
  const labels2: Record<number, string> = { [HEAD_ID]: pos === 1 ? 'prev' : 'head' }
  if (pos > 1) labels2[dataId(pos - 2)] = 'prev'
  if (succId != null) labels2[succId] = '后继'
  push(
    chain(seq2, i => (seq2[i].id === NEW_ID ? 'active' : 'none'), labels2),
    `④ prev->next = node：${pos === 1 ? '头结点' : `前驱 ${INIT[pos - 2]}`} 的 next 交给新结点，四条指针全部就位——选择题爱考这个数字`,
    null
  )

  // 完成帧
  push(
    chain(seq2, () => 'done'),
    `插入完成：head ⇄ ${seq2.slice(1).map(s => s.value).join(' ⇄ ')} → NULL，length ${n} → ${n + 1}；改 4 个指针域 = 新结点 prev/next + 前驱 next + 后继 prev`,
    null, seq2.map((_, i) => i)
  )

  return steps
}
