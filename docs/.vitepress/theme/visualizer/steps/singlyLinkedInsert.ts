// docs/.vitepress/theme/visualizer/steps/singlyLinkedInsert.ts
import type { Step } from '../types'
import type { LnNode, ListNodeState } from './states'

// 单链表按位插入（对应 01_线性表/03_单链表 singly_linked_list.c list_insert_at）
// 语义：带头结点；先从头结点走 pos-1 步定位第 pos-1 个结点 prev，
//       再 malloc 新结点，先接后继（node->next = prev->next），
//       再接前驱（prev->next = node）——顺序反了会断链
export interface SinglyLinkedInsertInput { pos: number; value: number }

const INIT = [12, 25, 33, 47, 58]   // 固定演示链表（页面已说明；输入只含"位序,值"）
const HEAD_ID = 0
const NEW_ID = 100
const dataId = (i: number) => i + 1 // 第 i 个数据结点（0 起）的 id

type Entry = { id: number; value: number | string }
const dataSeq = (): Entry[] => [{ id: HEAD_ID, value: 'head' }, ...INIT.map((v, i) => ({ id: dataId(i), value: v }))]

export function singlyLinkedInsertSteps(input: SinglyLinkedInsertInput): Step[] {
  const { pos, value } = input
  const n = INIT.length
  const steps: Step[] = []
  const push = (nodes: LnNode[], narration: string, active: number[] | null = null, highlights: number[] = [], note?: string) => {
    steps.push({ state: { nodes, headId: HEAD_ID, note } as ListNodeState, highlights, active, narration })
  }

  const chain = (seq: Entry[], hl: (i: number) => 'none' | 'active' | 'done', labels: Record<number, string> = {}): LnNode[] =>
    seq.map((s, i) => ({
      id: s.id, value: s.value,
      next: i + 1 < seq.length ? seq[i + 1].id : null,
      head: s.id === HEAD_ID,
      highlight: hl(i),
      label: labels[s.id]
    }))

  // 初始帧
  push(
    chain(dataSeq(), () => 'none'),
    `初始：带头结点单链表 head → ${INIT.join(' → ')} → NULL（length=${n}）；在第 ${pos} 位插入 ${value}，第一步从头结点出发定位第 ${pos - 1} 个结点（头结点算第 0 个）`
  )

  // 定位：走 pos-1 步（head 在 seq 下标 0，走 k 步后停在下标 k）
  if (pos === 1) {
    push(
      chain(dataSeq(), i => (i === 0 ? 'active' : 'none'), { [HEAD_ID]: 'prev' }),
      `pos = 1：第 0 个结点就是头结点，prev 原地不动——带头结点让头部插入与中部完全同一套代码，零特判`,
      [0]
    )
  } else {
    for (let k = 1; k <= pos - 1; k++) {
      const labels: Record<number, string> = {}
      labels[HEAD_ID] = '起点'
      for (let j = 1; j <= k; j++) labels[dataId(j - 1)] = j === k ? 'prev' : '已走'
      push(
        chain(dataSeq(), i => (i > 0 && i <= k ? (i === k ? 'active' : 'done') : 'none'), labels),
        `定位第 ${k}/${pos - 1} 步：prev 沿 next 走到第 ${k} 个结点（值 ${INIT[k - 1]}）${k < pos - 1 ? '，还没走到第 pos-1 个' : '——就是它，新结点将插在它之后'}`,
        [k]
      )
    }
  }

  // 新结点悬停（尚未接入）
  const hover: LnNode = { id: NEW_ID, value, next: null, highlight: 'none', label: '新结点' }
  push(
    [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none')), hover],
    `malloc 新结点（data=${value}, next=NULL）：它此刻还悬在链外；接下来两步的顺序是本动画的重点——先接后继，再接前驱`,
    null, [], `list_insert_at(list, ${pos}, ${value})`
  )

  // 第 1 步：先接后继（新结点仍悬在链外，画弧线牵住后继）
  const succId = pos <= n ? dataId(pos - 1) : null
  const labels1: Record<number, string> = {}
  if (pos <= n) labels1[dataId(pos - 1)] = '原后继'
  if (pos > 1) labels1[dataId(pos - 2)] = 'prev'
  push(
    [...chain(dataSeq(), i => (i > 0 && i < pos ? 'done' : 'none'), labels1), { ...hover, next: succId, highlight: 'active' }],
    `第 1 步 node->next = prev->next：新结点先接上后继 ${succId != null ? INIT[pos - 1] : 'NULL'}——此刻旧链未断，两头都牵着；若顺序反了，prev->next 先改，原后继的地址就丢了`,
    [n + 1]
  )

  // 第 2 步：再接前驱（新结点归位到链中：head 在下标 0，插在下标 pos 处）
  const seq2: Entry[] = dataSeq()
  seq2.splice(pos, 0, { id: NEW_ID, value })
  const labels2: Record<number, string> = { [HEAD_ID]: pos === 1 ? 'prev' : 'head' }
  if (pos > 1) labels2[dataId(pos - 2)] = 'prev'
  push(
    chain(seq2, i => (seq2[i].id === NEW_ID ? 'active' : 'none'), labels2),
    `第 2 步 prev->next = node：${pos === 1 ? '头结点' : `前驱 ${INIT[pos - 2]}`} 牵住新结点，链重新连续——插入本身只有这两次改指针，O(1)`
  )

  // 完成帧
  push(
    chain(seq2, () => 'done'),
    `插入完成：head → ${seq2.slice(1).map(s => s.value).join(' → ')} → NULL，length ${n} → ${n + 1}；定位第 pos-1 个 O(n) + 接链 O(1)，这就是"按位插入 O(n)、插删本身 O(1)"的构成`,
    null, seq2.map((_, i) => i)
  )

  return steps
}
