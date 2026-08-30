// docs/.vitepress/theme/visualizer/steps/blockStringChunk.ts
import type { Step } from '../types'

// 块链串分块存储（对应 05_串/03_块链串 block_string.c 的 bs_assign + append_char）
// 语义：CHUNK_SIZE=4 的结块串成单链表，尾块允许不满（无效槽不写不读，打印/演示
// 以 '#' 示意填充）；逻辑下标换算：第 i 个字符在第 i/4 块的第 i%4 槽。
export interface BlockChunkInput { src: string }
export const CHUNK_SIZE = 4 // 与 block_string.h 的 #define CHUNK_SIZE 一致

export function blockStringChunkSteps(input: BlockChunkInput): Step[] {
  const src = input.src
  const n = src.length
  const blocks: string[][] = []
  const frame = (narration: string, act?: { block: number; slot: number }, openBlocks: number[] = []): Step => ({
    state: {
      rows: blocks.map((b, bi) => {
        const chars = [...b]
        while (chars.length < CHUNK_SIZE) chars.push('#')
        const isLast = bi === blocks.length - 1
        return {
          label: `块 ${bi}${isLast ? '（尾块）' : ''} + next`,
          chars,
          charActive: act && act.block === bi ? [act.slot] : [],
          charHighlight: act && act.block === bi ? Array.from({ length: act.slot }, (_, k) => k) : (isLast ? undefined : Array.from({ length: CHUNK_SIZE }, (_, k) => k)),
          pointers: act && act.block === bi ? [{ name: 'tail', index: act.slot }] : (isLast ? [{ name: 'tail', index: Math.max(0, b.length - 1) }] : undefined)
        }
      }),
      note: `块数 ${blocks.length} × CHUNK_SIZE ${CHUNK_SIZE} = ${blocks.length * CHUNK_SIZE} 槽，有效 ${n} 个——存储密度 ${n}/${blocks.length * CHUNK_SIZE}${openBlocks.length ? `；本帧新开块：${openBlocks.join('、')}` : ''}`
    },
    highlights: [], active: null, narration
  })

  const steps: Step[] = [frame(
    `bs_assign 先 bs_clear 释放旧链再填充（失败不留半截数据）；CHUNK_SIZE = ${CHUNK_SIZE}：结块越大存储密度越高、结点开销越小——这是"密度 vs 指针开销"的权衡参数`)]

  for (let i = 0; i < n; i++) {
    const used = i % CHUNK_SIZE
    const opened: number[] = []
    if (used === 0) {
      blocks.push([])
      opened.push(blocks.length - 1)
      steps.push(frame(`append_char：${i === 0 ? '空串' : '尾块已满（used = length % 4 = 0）'}→ malloc 开新块（第 ${blocks.length - 1} 块），tail 指向它——尾插 O(1) 靠 head/tail 双指针`))
    }
    blocks[blocks.length - 1].push(src[i])
    steps.push(frame(`写入第 ${i + 1} 个字符 '${src[i]}'：逻辑位 i=${i} → 第 ${Math.floor(i / CHUNK_SIZE)} 块第 ${used} 槽（换算 i/4 与 i%4），length = ${i + 1}`,
      { block: blocks.length - 1, slot: used }, opened))
  }

  steps.push(frame(
    `赋值成功：${n} 个字符 = ${blocks.length} 个结块；尾块有效槽数 = length % ${CHUNK_SIZE} = ${n % CHUNK_SIZE || CHUNK_SIZE}${n % CHUNK_SIZE ? `，剩余 ${CHUNK_SIZE - (n % CHUNK_SIZE)} 个无效槽以 '#' 示意（实现不写入填充符，以 length 管理有效性）` : '（恰好装满）'}；bs_print 以 '|' 画块边界，bs_get 定位第 i 块要沿链走 O(n/4)`))

  return steps
}
