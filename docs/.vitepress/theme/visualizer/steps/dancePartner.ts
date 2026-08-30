// docs/.vitepress/theme/visualizer/steps/dancePartner.ts
import type { Step } from '../types'
import type { StackQueueState } from './states'

// 舞伴配对（对应 03_队列/03_队列的应用/01_舞伴问题 dance_partner.c）
// 语义：三阶段——①按性别分队（到达顺序即入队顺序，源码 sex=='M' 入男队、'F' 入女队，
//       用数组 + head/tail 线性下标实现 enqueue/dequeue）；
//       ②两队队头同时出队配对（循环条件两队都不空，&&，先到先配）；
//       ③任一队空即停，剩余者从队头到队尾依次报告
export interface DancePartnerInput { sexes: string[] }   // 每位 'M'（男）或 'F'（女）

export function dancePartnerSteps(input: DancePartnerInput): Step[] {
  const { sexes } = input
  const males = sexes.filter(s => s === 'M')
  const females = sexes.filter(s => s === 'F')
  const mLabel = (i: number) => `男${i + 1}`
  const fLabel = (i: number) => `女${i + 1}`

  const steps: Step[] = []
  const state = (maleCells: (number | string)[], femaleCells: (number | string)[], mh: number, fh: number) => ({
    slots: [
      { label: '男队（M，先到先配）', kind: 'queue' as const, items: maleCells, pointers: [{ name: 'head', index: mh }, { name: 'tail', index: males.length }] },
      { label: '女队（F，先到先配）', kind: 'queue' as const, items: femaleCells, pointers: [{ name: 'head', index: fh }, { name: 'tail', index: females.length }] }
    ]
  })
  const pushStep = (maleCells: (number | string)[], femaleCells: (number | string)[], mh: number, fh: number, narration: string, active: number[] | null = null, highlights: number[] = []) => {
    steps.push({ state: state(maleCells, femaleCells, mh, fh) as StackQueueState, highlights, active, narration })
  }

  pushStep(new Array(males.length).fill(''), new Array(females.length).fill(''), 0, 0,
    `阶段一 分队：${sexes.length} 位到场者按性别入队（到达顺序即入队顺序，源码 sex=='M' → male_queue[tail++]、'F' → female_queue[tail++]）；下面逐个入队`)

  // 阶段一：逐个入队
  let mi = 0, fi = 0
  const mCells: (number | string)[] = new Array(males.length).fill('')
  const fCells: (number | string)[] = new Array(females.length).fill('')
  sexes.forEach((s, i) => {
    if (s === 'M') {
      mCells[mi] = mLabel(mi)
      pushStep([...mCells], [...fCells], 0, 0, `第 ${i + 1} 位到场：男宾 → 入男队尾（male_tail++），FIFO 保证先到的先配`, [mi])
      mi++
    } else {
      fCells[fi] = fLabel(fi)
      pushStep([...mCells], [...fCells], 0, 0, `第 ${i + 1} 位到场：女宾 → 入女队尾（female_tail++）`, [males.length + fi])
      fi++
    }
  })

  // 阶段二：队头两两配对
  let mh = 0, fh = 0
  let pairs = 0
  while (mh < males.length && fh < females.length) {
    pairs++
    pushStep([...mCells], [...fCells], mh, fh,
      `第 ${pairs} 对：${mLabel(mh)} ↔ ${fLabel(fh)}（两队队头同时出队，male_head++、female_head++——循环条件 male_head < male_tail && female_head < female_tail，任一队先空即停）`,
      [mh, males.length + fh])
    mh++
    fh++
  }

  // 阶段三：剩余者
  const leftM = males.length - mh
  const leftF = females.length - fh
  const doneM = mCells.map((_, i) => i).filter(i => i < mh)
  const doneF = fCells.map((_, i) => i).filter(i => i < fh).map(i => i + males.length)
  pushStep([...mCells], [...fCells], mh, fh,
    leftM + leftF === 0
      ? `配对结束：共 ${pairs} 对，所有人都配到了舞伴`
      : `配对结束：共 ${pairs} 对（= 较少一方人数）；${leftM > 0 ? `男队剩 ${leftM} 人（${Array.from({ length: leftM }, (_, k) => mLabel(mh + k)).join('、')}）` : ''}${leftM > 0 && leftF > 0 ? '、' : ''}${leftF > 0 ? `女队剩 ${leftF} 人（${Array.from({ length: leftF }, (_, k) => fLabel(fh + k)).join('、')}）` : ''}等下一支舞曲——剩余者从队头到队尾依次报告，越早到的越先等到`,
    null, [...doneM, ...doneF])

  return steps
}
