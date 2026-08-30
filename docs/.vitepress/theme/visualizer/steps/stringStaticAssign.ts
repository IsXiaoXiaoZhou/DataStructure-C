// docs/.vitepress/theme/visualizer/steps/stringStaticAssign.ts
import type { Step } from '../types'

// 定长顺序串 assign（对应 05_串/01_定长顺序串 static_string.c 的 ss_assign）
// 语义：先查容量再写入——n > STR_MAX_SIZE(100) 直接 DS_OVERFLOW 报错不截断；
// 逐字符 s->data[i] = cstr[i] 拷入，length 随之递增。动画演示缓冲画 12 格。
export interface StaticAssignInput { src: string }
const DEMO_CAP = 12

export function stringStaticAssignSteps(input: StaticAssignInput): Step[] {
  const src = input.src
  const n = src.length
  const dst: string[] = []
  const frame = (len: number, narration: string, hi: number[] = [], act: number[] = []): Step => ({
    state: {
      rows: [
        { label: `源串 cstr（${n} 个字符）`, chars: src.split(''), charHighlight: [...hi], charActive: [...act] },
        { label: `目的串 s->data（演示容量 ${DEMO_CAP}）length=${len}`, chars: [...dst], capacity: DEMO_CAP, charHighlight: [...hi], charActive: [...act], pointers: [{ name: 'i', index: act.length ? act[0] : Math.min(len, DEMO_CAP - 1) }] }
      ]
    },
    highlights: [], active: null, narration
  })

  const steps: Step[] = [frame(0,
    `ss_assign 前置检查：本实现 n=${n} ≤ STR_MAX_SIZE=100，容量检查通过——源码对超长串返回 DS_OVERFLOW"赋值超长"报错而非静默截断（教材的截断策略会留下脏数据）`)]

  for (let i = 0; i < n; i++) {
    dst.push(src[i])
    steps.push(frame(i + 1,
      `拷入第 ${i + 1} 个字符：s->data[${i}] = cstr[${i}] = '${src[i]}'，length 递增到 ${i + 1}`,
      Array.from({ length: i }, (_, k) => k), [i]))
  }

  steps.push(frame(n,
    `赋值成功（DS_OK）：length = ${n}，length 域自管理、'\\0' 不入串——打印按长度输出，不依赖终止符；整串复制 O(n)，之后 ss_copy 同款逐位搬运`))

  return steps
}
