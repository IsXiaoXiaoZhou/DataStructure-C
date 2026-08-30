// docs/.vitepress/theme/visualizer/steps/bracketMatch.ts
import type { Step } from '../types'
import type { StackQueueState } from './states'

// 括号匹配（对应 02_栈/04_栈的应用/01_括号匹配 bracket_match.c）
// 语义：逐字符扫描；左括号连同下标入栈"期待配对"；右括号必须与栈顶同类，
//       配对弹出（top--）；栈空遇右括号 → BRACKET_MISSING_LEFT（报右括号下标）；
//       不同类 → BRACKET_MISMATCH（报右括号下标）；扫描结束栈非空 →
//       BRACKET_MISSING_RIGHT（报栈底即最早未配对左括号的下标）；栈空 → BRACKET_OK。
//       三种错误一出现即返回，与源码一致
export interface BracketMatchInput { expr: string }

const PAIR: Record<string, string> = { '(': ')', '[': ']', '{': '}' }
const LEFT = '([{'
const RIGHT = ')]}'

export function bracketMatchSteps(input: BracketMatchInput): Step[] {
  const expr = input.expr
  const steps: Step[] = []
  const SCAN = expr.split('')

  const state = (stack: string[], top: number) => ({
    slots: [
      { label: '扫描序列（i 从 0 起）', kind: 'queue' as const, items: [...SCAN] },
      { label: '括号栈（左括号 + 下标等待配对）', kind: 'stack' as const, capacity: 8, items: [...stack], pointers: [{ name: 'top', index: top }] }
    ]
  })
  const pushStep = (stack: string[], top: number, narration: string, active: number[] | null = null, highlights: number[] = []) => {
    steps.push({ state: state(stack, top) as StackQueueState, highlights, active, narration })
  }

  pushStep([], -1, `初始：栈空（top = -1）；逐字符扫描 "${expr}"——左括号入栈等待配对，右括号必须与栈顶同类，非括号字符与匹配无关`)

  const stack: string[] = []
  const stackPos: number[] = []   // 栈内各左括号在表达式中的下标（源码 BracketItem.pos）
  let stopped = false

  for (let i = 0; i < expr.length; i++) {
    const ch = expr[i]
    if (LEFT.includes(ch)) {
      stack.push(ch)
      stackPos.push(i)
      pushStep([...stack], stack.length - 1, `i=${i}：'${ch}' 是左括号 → 连同下标入栈，期待配对 '${PAIR[ch]}'`, [i, SCAN.length + stack.length - 1])
    } else if (RIGHT.includes(ch)) {
      if (stack.length === 0) {
        pushStep([...stack], -1, `i=${i}：'${ch}' 是右括号但栈已空——它前面没有左括号在等它，报 BRACKET_MISSING_LEFT @ i=${i}（检查完成，缺左括号）`, [i])
        stopped = true
        break
      }
      const top = stack[stack.length - 1]
      if (PAIR[top] !== ch) {
        pushStep([...stack], stack.length - 1, `i=${i}：'${ch}' 与栈顶 '${top}' 不同类（栈顶在等 '${PAIR[top]}'），报 BRACKET_MISMATCH @ i=${i}（检查完成，括号类型不匹配）`, [i, SCAN.length + stack.length - 1])
        stopped = true
        break
      }
      stack.pop()
      stackPos.pop()
      pushStep([...stack], stack.length - 1, `i=${i}：'${ch}' 与栈顶 '${top}' 同类 → 弹出配对（top--）`, i >= 0 ? [i] : [], stack.length > 0 ? [SCAN.length + stack.length - 1] : [])
    }
  }

  if (!stopped) {
    if (stack.length === 0) {
      pushStep([], -1, `扫描结束：栈空（top == -1）→ 检查完成，括号完全匹配（BRACKET_OK）`, null, SCAN.map((_, i) => i))
    } else {
      pushStep([...stack], stack.length - 1, `扫描结束：栈非空——最早入栈的 '${stack[0]}'（i=${stackPos[0]}，报栈底符合"从头找第一个错"的阅读直觉）始终没等到右括号，报 BRACKET_MISSING_RIGHT（检查完成，缺右括号）`, [SCAN.length])
    }
  }

  return steps
}
