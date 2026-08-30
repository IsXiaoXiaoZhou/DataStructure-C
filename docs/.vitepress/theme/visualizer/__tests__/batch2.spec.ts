// docs/.vitepress/theme/visualizer/__tests__/batch2.spec.ts
// 批2：StackQueue / ListNode 双渲染器的 10 个动画（栈 / 队列 / 链表）
import { describe, it, expect } from 'vitest'
import type { Step } from '../types'
import { registry } from '../registry'

import { singlyLinkedInsertSteps } from '../steps/singlyLinkedInsert'
import { doublyLinkedInsertSteps } from '../steps/doublyLinkedInsert'
import { josephusRingSteps } from '../steps/josephusRing'
import { seqStackSteps } from '../steps/seqStack'
import { sharedStackSteps } from '../steps/sharedStack'
import { linkStackSteps } from '../steps/linkStack'
import { bracketMatchSteps } from '../steps/bracketMatch'
import { circularQueueSteps } from '../steps/circularQueue'
import { linkQueueSteps } from '../steps/linkQueue'
import { dancePartnerSteps } from '../steps/dancePartner'

const clone = <T,>(x: T): T => structuredClone(x)
const last = (steps: Step[]): Step => steps[steps.length - 1]
const allNarration = (steps: Step[]) => steps.map(s => s.narration).join('\n')

/** 纯函数：同输入两次运行结果一致，且不改输入 */
function expectPure<T>(fn: (x: T) => Step[], input: T) {
  const before = clone(input)
  const a = fn(input)
  const b = fn(input)
  expect(input).toEqual(before)   // 不修改输入
  expect(a).toEqual(b)            // 输出确定性
}

/** StackQueue state 契约：slots 形状、指针下标 ∈ [-1, items.length]、全局展平高亮不越界 */
function expectSqContract(steps: Step[]) {
  expect(steps.length).toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length, 'narration 非空').toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.slots)).toBe(true)
    expect(o.slots.length).toBeGreaterThan(0)
    let total = 0
    for (const slot of o.slots) {
      expect(typeof slot.label).toBe('string')
      expect(Array.isArray(slot.items)).toBe(true)
      if (slot.capacity != null) expect(slot.items.length).toBeLessThanOrEqual(slot.capacity)
      for (const p of slot.pointers ?? []) {
        expect(typeof p.name).toBe('string')
        expect(Number.isInteger(p.index)).toBe(true)
        expect(p.index).toBeGreaterThanOrEqual(-1)
        expect(p.index).toBeLessThanOrEqual(slot.items.length)
      }
      total += slot.items.length
    }
    for (const i of [...s.highlights, ...(s.active ?? [])]) {
      expect(i).toBeGreaterThanOrEqual(0)
      expect(i).toBeLessThan(total)
    }
  }
}

/** ListNode state 契约：id 唯一、next/prev 引用可解析、headId 合法、highlight 三态、高亮按 nodes 下标不越界 */
function expectLnContract(steps: Step[]) {
  expect(steps.length).toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length, 'narration 非空').toBeGreaterThan(0)
    const o = s.state as any
    expect(Array.isArray(o.nodes)).toBe(true)
    expect(o.headId === null || typeof o.headId === 'number').toBe(true)
    const ids = new Set<number>(o.nodes.map((n: any) => n.id))
    expect(ids.size).toBe(o.nodes.length)
    for (const n of o.nodes) {
      expect(typeof n.id).toBe('number')
      if (n.next != null) expect(ids.has(n.next), `结点 ${n.id} 的 next 悬空`).toBe(true)
      if (n.prev != null) expect(ids.has(n.prev), `结点 ${n.id} 的 prev 悬空`).toBe(true)
      expect(['none', 'active', 'done', undefined]).toContain(n.highlight)
    }
    if (o.headId !== null) expect(ids.has(o.headId)).toBe(true)
    for (const i of [...s.highlights, ...(s.active ?? [])]) {
      expect(i).toBeGreaterThanOrEqual(0)
      expect(i).toBeLessThan(o.nodes.length)
    }
  }
}

/** 沿 next 从 headId 走链，返回值序列（防环死循环，带步数上限） */
function chainValues(o: any): (number | string)[] {
  const byId = new Map(o.nodes.map((n: any) => [n.id, n]))
  const out: (number | string)[] = []
  let cur = o.headId
  let guard = 0
  while (cur != null && guard++ <= o.nodes.length + 1) {
    const n = byId.get(cur)
    if (!n) break
    if (!n.head) out.push(n.value)
    cur = n.next ?? null
    if (cur === o.headId) break   // 环回到起点
  }
  return out
}

// ---------- 1. singly-linked-insert 单链表按位插入 ----------
describe('singlyLinkedInsert 生成器', () => {
  const input = { pos: 3, value: 40 }
  it('终帧：链序 head→12→25→40→33→47→58→NULL，全部 done，narration 含完成', () => {
    const steps = singlyLinkedInsertSteps(input)
    const fin = last(steps)
    expect(chainValues(fin.state as any)).toEqual([12, 25, 40, 33, 47, 58])
    expect(fin.highlights).toHaveLength(7)          // head + 6 数据结点
    expect(fin.narration).toContain('完成')
    expectLnContract(steps)
  })
  it('新结点有悬停帧，且 narration 强调先接后继再接前驱的顺序', () => {
    const steps = singlyLinkedInsertSteps(input)
    expect(steps.some(s => (s.state as any).nodes.some((n: any) => n.label === '新结点' && n.next === null))).toBe(true)
    expect(allNarration(steps)).toContain('先接上后继')
    expect(allNarration(steps)).toContain('顺序反了')
  })
  it('位序 1 走头结点零特判分支；表尾追加后继为 NULL', () => {
    const head = singlyLinkedInsertSteps({ pos: 1, value: 7 })
    expect(allNarration(head)).toContain('零特判')
    expect(chainValues(last(head).state as any)).toEqual([7, 12, 25, 33, 47, 58])
    const tail = singlyLinkedInsertSteps({ pos: 6, value: 99 })
    expect(chainValues(last(tail).state as any)).toEqual([12, 25, 33, 47, 58, 99])
    expect(allNarration(tail)).toContain('NULL')
  })
  it('纯函数', () => { expectPure(singlyLinkedInsertSteps, input) })
})

// ---------- 2. doubly-linked-insert 双链表按位插入 ----------
describe('doublyLinkedInsert 生成器', () => {
  const input = { pos: 3, value: 40 }
  it('终帧：双向链序一致（next 正向 / prev 反向），narration 含完成与四条指针', () => {
    const steps = doublyLinkedInsertSteps(input)
    const fin = last(steps)
    expect(chainValues(fin.state as any)).toEqual([12, 25, 40, 33, 47, 58])
    // prev 反向走：从最后一个数据结点沿 prev 回到 head
    const nodes = (fin.state as any).nodes as any[]
    const byId = new Map(nodes.map(n => [n.id, n]))
    let cur = nodes.find(n => n.next === null)
    const rev: any[] = []
    while (cur && !cur.head) { rev.push(cur.value); cur = byId.get(cur.prev) }
    expect(rev).toEqual([58, 47, 33, 40, 25, 12])
    expect(fin.narration).toContain('完成')
    expect(fin.narration).toContain('4 个指针域')
    expectLnContract(steps)
  })
  it('四条指针逐帧各有交代（①②③④），后继回指帧存在', () => {
    const steps = doublyLinkedInsertSteps(input)
    const all = allNarration(steps)
    for (const m of ['① node->prev', '② node->next', '③ prev->next->prev', '④ prev->next']) {
      expect(all).toContain(m)
    }
  })
  it('表尾插入走 tail 改指分支（③ 后继为 NULL）', () => {
    const steps = doublyLinkedInsertSteps({ pos: 6, value: 99 })
    expect(allNarration(steps)).toContain('tail 改指新结点')
    expect(chainValues(last(steps).state as any)).toEqual([12, 25, 33, 47, 58, 99])
  })
  it('纯函数', () => { expectPure(doublyLinkedInsertSteps, input) })
})

// ---------- 3. josephus-ring 约瑟夫环 ----------
describe('josephusRing 生成器', () => {
  const input = { n: 8, start: 1, m: 3 }
  it('n=8, m=3, start=1：出列序列与 main.c 断言前 7 位一致（3 6 1 5 2 8 4），剩 7 号收尾', () => {
    const steps = josephusRingSteps(input)
    const outs = steps.map(s => s.narration.match(/出列的是 (\d+) 号/)).filter(Boolean).map(m => Number(m![1]))
    expect(outs).toEqual([3, 6, 1, 5, 2, 8, 4])   // 动画演到剩最后一人（main.c 会继续报到圈空）
    expect(last(steps).narration).toContain('只剩 7 号')
    expectLnContract(steps)
  })
  it('每帧尾结点 ring 标记成立，next 首尾相接成环（未被跨接的帧）', () => {
    const steps = josephusRingSteps(input)
    for (const s of steps) {
      const o = s.state as any
      if (o.nodes.length > 1 && !s.narration.includes('越过')) {
        const tail = o.nodes[o.nodes.length - 1]
        expect(tail.ring).toBe(true)
        expect(tail.next).toBe(o.nodes[0].id)
      }
    }
  })
  it('报数逐节点高亮帧 narration 给出报数进度；跨接帧交代前驱', () => {
    const steps = josephusRingSteps(input)
    expect(allNarration(steps)).toContain('前驱')
    expect(steps.filter(s => /报数 \d+\/\d+/.test(s.narration)).length).toBeGreaterThan(0)
  })
  it('纯函数', () => { expectPure(josephusRingSteps, input) })
})

// ---------- 4. seq-stack 顺序栈 ----------
describe('seqStack 生成器', () => {
  const input = { ops: [{ op: 'push', v: 5 }, { op: 'push', v: 2 }, { op: 'pop' }, { op: 'push', v: 7 }] }
  it('终帧：栈内自底向上 [5, 7]，top=1；narration 含演示结束', () => {
    const steps = seqStackSteps(input)
    const o = last(steps).state as any
    expect(o.slots[0].items).toEqual([5, 7])
    expect(o.slots[0].pointers[0]).toEqual({ name: 'top', index: 1 })
    expect(last(steps).narration).toContain('演示结束')
    expectSqContract(steps)
  })
  it('满栈 push 报 DS_OVERFLOW（status=4），空栈 pop 报 DS_EMPTY（status=5）', () => {
    const full = seqStackSteps({ ops: Array.from({ length: 7 }, (_, i) => ({ op: 'push', v: i + 1 })) })
    expect(allNarration(full)).toContain('DS_OVERFLOW')
    expect(allNarration(full)).toContain('status=4')
    const empty = seqStackSteps({ ops: [{ op: 'pop' }] })
    expect(empty[empty.length - 2].narration).toContain('DS_EMPTY')
    expect(empty[empty.length - 2].narration).toContain('status=5')
  })
  it('narration 交代 top 先自增再存值 / 先取值再下移', () => {
    const all = allNarration(seqStackSteps(input))
    expect(all).toContain('top 先自增')
    expect(all).toContain('top 再下移')
  })
  it('纯函数', () => { expectPure(seqStackSteps, input) })
})

// ---------- 5. shared-stack 两栈共享空间 ----------
describe('sharedStack 生成器', () => {
  const input = { ops: [{ id: 1, op: 'push', v: 5 }, { id: 2, op: 'push', v: 9 }, { id: 1, op: 'push', v: 3 }, { id: 2, op: 'pop' }] }
  it('终帧：data[0..1]=5,3（栈1）、top2 回到 MAX=8；narration 给出两侧计数', () => {
    const steps = sharedStackSteps(input)
    const o = last(steps).state as any
    const slot = o.slots[0]
    expect(slot.items.slice(0, 2)).toEqual([5, 3])
    expect(slot.items[7]).toBe('')                       // 9 已弹出
    expect(slot.pointers.map((p: any) => p.name)).toEqual(['top1', 'top2'])
    expect(slot.pointers[0].index).toBe(1)
    expect(slot.pointers[1].index).toBe(8)               // 栈2 空 = MAX（渲染器画在槽外）
    expect(last(steps).narration).toContain('演示结束')
    expect(last(steps).narration).toContain('空闲 6 格')
    expectSqContract(steps)
  })
  it('判满 top1 + 1 == top2 报 DS_OVERFLOW；两栈判空各自报 DS_EMPTY', () => {
    const ops: any[] = []
    for (let i = 0; i < 4; i++) { ops.push({ id: 1, op: 'push', v: i + 1 }); ops.push({ id: 2, op: 'push', v: i + 1 }) }
    ops.push({ id: 1, op: 'push', v: 9 })             // 8 格已占满，第 9 个 push 触发判满
    const full = sharedStackSteps({ ops })
    const err = full[full.length - 2]
    expect(err.narration).toContain('top1 + 1 == top2')
    expect(err.narration).toContain('DS_OVERFLOW')
    expect(err.narration).toContain('3 + 1 == 4')
    const e1 = sharedStackSteps({ ops: [{ id: 1, op: 'pop' }] })
    expect(e1[e1.length - 2].narration).toContain('栈1 空（top1 == -1）')
    const e2 = sharedStackSteps({ ops: [{ id: 2, op: 'pop' }] })
    expect(e2[e2.length - 2].narration).toContain('栈2 空（top2 == 8）')
  })
  it('narration 交代栈1 向右 / 栈2 向左的增长方向', () => {
    const all = allNarration(sharedStackSteps(input))
    expect(all).toContain('向右端扩张')
    expect(all).toContain('向左端扩张')
  })
  it('纯函数', () => { expectPure(sharedStackSteps, input) })
})

// ---------- 6. link-stack 链栈 ----------
describe('linkStack 生成器', () => {
  const input = { ops: [{ op: 'push', v: 5 }, { op: 'push', v: 2 }, { op: 'push', v: 9 }, { op: 'pop' }, { op: 'pop' }] }
  it('终帧：自栈顶向下 [5]（9、2 已弹），headId 指向栈顶', () => {
    const steps = linkStackSteps(input)
    const o = last(steps).state as any
    expect(o.headLabel).toBe('top')
    expect(chainValues(o)).toEqual([5])
    expect(o.nodes[0].id).toBe(o.headId)
    expect(last(steps).narration).toContain('演示结束')
    expectLnContract(steps)
  })
  it('头插两步各有帧；空栈 pop 报 DS_EMPTY（status=5）', () => {
    const steps = linkStackSteps(input)
    const all = allNarration(steps)
    expect(all).toContain('node->next = top')
    expect(all).toContain('top = node')
    const empty = linkStackSteps({ ops: [{ op: 'push', v: 1 }, { op: 'pop' }, { op: 'pop' }] })
    const err = empty[empty.length - 2]
    expect(err.narration).toContain('DS_EMPTY')
    expect(err.narration).toContain('top == NULL')
  })
  it('push 前有新结点悬停帧（label=新结点）', () => {
    const steps = linkStackSteps(input)
    expect(steps.some(s => (s.state as any).nodes.some((n: any) => n.label === '新结点'))).toBe(true)
  })
  it('纯函数', () => { expectPure(linkStackSteps, input) })
})

// ---------- 7. bracket-match 括号匹配 ----------
describe('bracketMatch 生成器', () => {
  const input = { expr: '{[()]}([)]' }
  it('"{[()]}([)]"：i=8 处 ) 与栈顶 [ 不同类 → BRACKET_MISMATCH 即停', () => {
    const steps = bracketMatchSteps(input)
    expect(last(steps).narration).toContain('BRACKET_MISMATCH')
    expect(last(steps).narration).toContain('i=8')
    expect(steps.length).toBeLessThan(12)                // 报错即返回，不扫完
    expectSqContract(steps)
  })
  it('成功 / 缺左 / 缺右三种终局各有对应 BRACKET_* 状态', () => {
    const ok = bracketMatchSteps({ expr: '{[()]}' })
    expect(last(ok).narration).toContain('BRACKET_OK')
    expect(last(ok).narration).toContain('完全匹配')
    const missL = bracketMatchSteps({ expr: ')(' })
    expect(last(missL).narration).toContain('BRACKET_MISSING_LEFT')
    expect(last(missL).narration).toContain('i=0')
    const missR = bracketMatchSteps({ expr: '{(' })
    expect(last(missR).narration).toContain('BRACKET_MISSING_RIGHT')
    expect(last(missR).narration).toContain("i=0")       // 报栈底（最早未配对）
  })
  it('state 恒为双槽：扫描序列 + 括号栈；左括号入栈帧交代"期待配对"', () => {
    const steps = bracketMatchSteps(input)
    for (const s of steps) expect((s.state as any).slots).toHaveLength(2)
    expect(allNarration(steps)).toContain('期待配对')
  })
  it('纯函数', () => { expectPure(bracketMatchSteps, input) })
})

// ---------- 8. circular-queue 循环顺序队列 ----------
describe('circularQueue 生成器', () => {
  const input = { ops: [{ op: 'en', v: 5 }, { op: 'en', v: 2 }, { op: 'en', v: 7 }, { op: 'de' }, { op: 'en', v: 9 }, { op: 'en', v: 3 }, { op: 'en', v: 4 }, { op: 'de' }] }
  it("终帧：data = ['', '', 7, 9, 3, 4]，front=2 rear=0，narration 给出元素个数算式 = 4", () => {
    const steps = circularQueueSteps(input)
    const o = last(steps).state as any
    const slot = o.slots[0]
    expect(slot.kind).toBe('ring')
    expect(slot.items).toEqual(['', '', 7, 9, 3, 4])
    const front = slot.pointers.find((p: any) => p.name === 'front')
    const rear = slot.pointers.find((p: any) => p.name === 'rear')
    expect(front.index).toBe(2)
    expect(rear.index).toBe(0)
    expect(last(steps).narration).toContain('= 4')
    expectSqContract(steps)
  })
  it('入队 narration 给出取模算式 rear=(rear+1)%6；绕回帧出现', () => {
    const all = allNarration(circularQueueSteps(input))
    expect(all).toContain('rear = (rear+1)%6 = (5+1)%6 = 0')
    expect(all).toContain('绕回')
  })
  it('判满牺牲一格：(rear+1)%6==front 报 DS_OVERFLOW；空队 de 报 DS_EMPTY', () => {
    const full = circularQueueSteps({ ops: [{ op: 'en', v: 1 }, { op: 'en', v: 2 }, { op: 'en', v: 3 }, { op: 'en', v: 4 }, { op: 'en', v: 5 }, { op: 'en', v: 6 }] })
    const err = full[full.length - 2]
    expect(err.narration).toContain('DS_OVERFLOW')
    expect(err.narration).toContain('(5+1)%6 = 0')
    const empty = circularQueueSteps({ ops: [{ op: 'de' }] })
    expect(empty[empty.length - 2].narration).toContain('DS_EMPTY')
  })
  it('纯函数', () => { expectPure(circularQueueSteps, input) })
})

// ---------- 9. link-queue 链队列 ----------
describe('linkQueue 生成器', () => {
  const input = { ops: [{ op: 'en', v: 5 }, { op: 'en', v: 2 }, { op: 'de' }, { op: 'en', v: 7 }] }
  it('终帧：队列 [2, 7]，headId 恒指头结点，narration 含演示结束', () => {
    const steps = linkQueueSteps(input)
    const o = last(steps).state as any
    expect(chainValues(o)).toEqual([2, 7])
    expect(o.headId).toBe(0)
    expect(o.nodes[0].head).toBe(true)
    expect(o.nodes[o.nodes.length - 1].label).toBe('rear')
    expect(last(steps).narration).toContain('演示结束')
    expectLnContract(steps)
  })
  it('删空回位特殊帧：删最后一个结点时 narration 交代 rear = front', () => {
    const steps = linkQueueSteps({ ops: [{ op: 'en', v: 5 }, { op: 'de' }, { op: 'en', v: 7 }] })
    const all = allNarration(steps)
    expect(all).toContain('rear = front 拉回头结点')
    expect(all).toContain('野指针')
    expect(last(steps).narration).toContain('[7]')
  })
  it('空队出队报 DS_EMPTY；头结点 label 在空队时为 front=rear', () => {
    const steps = linkQueueSteps({ ops: [{ op: 'en', v: 5 }, { op: 'de' }, { op: 'de' }] })
    const err = steps[steps.length - 2]
    expect(err.narration).toContain('DS_EMPTY')
    const o = last(steps).state as any
    expect(o.nodes[0].label).toBe('front=rear')
    expect(o.nodes[0].next).toBeNull()
  })
  it('纯函数', () => { expectPure(linkQueueSteps, input) })
})

// ---------- 10. dance-partner 舞伴配对 ----------
describe('dancePartner 生成器', () => {
  const input = { sexes: ['F', 'M', 'F', 'F', 'M', 'M', 'F'] }
  it('终帧：3 对配对（M=男队 3 人先耗尽），女队剩 1 人等下一支舞曲', () => {
    const steps = dancePartnerSteps(input)
    const fin = last(steps)
    expect(fin.narration).toContain('共 3 对')
    expect(fin.narration).toContain('女队剩 1 人')
    expect(fin.narration).toContain('女4')
    const o = fin.state as any
    expect(o.slots).toHaveLength(2)
    expect(o.slots[0].items).toEqual(['男1', '男2', '男3'])
    expect(o.slots[1].items).toEqual(['女1', '女2', '女3', '女4'])
    expectSqContract(steps)
  })
  it('配对帧 active 高亮两队队头（全局展平索引），narration 给出配对男女', () => {
    const steps = dancePartnerSteps(input)
    const pairFrames = steps.filter(s => s.narration.includes('↔'))
    expect(pairFrames).toHaveLength(3)
    expect(pairFrames[0].narration).toContain('男1 ↔ 女1')
    expect(pairFrames[0].active).toEqual([0, 3])       // 男队头 0 + 女队头（偏移 3）
    expect(pairFrames[2].narration).toContain('男3 ↔ 女3')
  })
  it('全部同性的极端输入：0 对、全员等待', () => {
    const steps = dancePartnerSteps({ sexes: ['M', 'M'] })
    expect(last(steps).narration).toContain('共 0 对')
    expect(last(steps).narration).toContain('男队剩 2 人')
  })
  it('纯函数', () => { expectPure(dancePartnerSteps, input) })
})

// ---------- registry：批2 新增键 ----------
const B2_KEYS = [
  'singly-linked-insert', 'doubly-linked-insert', 'josephus-ring',
  'seq-stack', 'shared-stack', 'link-stack', 'bracket-match',
  'circular-queue', 'link-queue', 'dance-partner'
]

describe('registry 批2 新增键', () => {
  it('10 个新键全部注册、renderer 正确、defaultInput 通过自身校验', () => {
    const listNode = new Set(['singly-linked-insert', 'doubly-linked-insert', 'josephus-ring', 'link-stack', 'link-queue'])
    for (const k of B2_KEYS) {
      const def = registry[k]
      expect(def, `缺键 ${k}`).toBeTruthy()
      expect(def.renderer, `${k} renderer`).toBe(listNode.has(k) ? 'listNode' : 'stackQueue')
      expect(def.validate(def.parse(def.defaultInput))).toBeNull()
    }
    expect(Object.keys(registry)).toHaveLength(47)
  })
  it('全链路：defaultInput 经 parse→validate→steps 产出有效帧（PlayerShell 同款调用序）', () => {
    for (const k of B2_KEYS) {
      const def = registry[k]
      const parsed = def.parse(def.defaultInput)
      const steps = def.steps(parsed)
      expect(steps.length, `${k} 帧数不足`).toBeGreaterThan(1)
      for (const s of steps) expect(s.narration.length, `${k} 空 narration`).toBeGreaterThan(0)
    }
  })
  it('非法输入返回中文提示（含示例字样"如"）', () => {
    expect(registry['singly-linked-insert'].validate(registry['singly-linked-insert'].parse('7,40'))).toContain('位序')
    expect(registry['singly-linked-insert'].validate(registry['singly-linked-insert'].parse('3'))).toContain('如')
    expect(registry['josephus-ring'].validate(registry['josephus-ring'].parse('8,9,3'))).toContain('起始')
    expect(registry['seq-stack'].validate(registry['seq-stack'].parse('push:5,pop:3'))).toContain('如')
    expect(registry['shared-stack'].validate(registry['shared-stack'].parse('3:push:5'))).toContain('如')
    expect(registry['bracket-match'].validate(registry['bracket-match'].parse('{[()]}a'))).toContain('括号字符')
    expect(registry['circular-queue'].validate(registry['circular-queue'].parse('enq:5'))).toContain('如')
    expect(registry['dance-partner'].validate(registry['dance-partner'].parse('F,X'))).toContain('M（男）')
  })
})
