// docs/.vitepress/theme/visualizer/__tests__/sortA.spec.ts
import { describe, it, expect } from 'vitest'
import { insertionSortSteps } from '../steps/insertionSort'
import { shellSortSteps } from '../steps/shellSort'
import { bubbleSortSteps } from '../steps/bubbleSort'
import { quickSortSteps } from '../steps/quickSort'
import { expectSortedSteps, expectFrameInvariants } from './steps.common'

const cases: [string, (a: number[]) => any, number[][]][] = [
  ['insertion', insertionSortSteps, [[5, 2, 9, 1, 7], [1], [3, 3, 1], [4, 3, 2, 1]]],
  ['shell', shellSortSteps, [[8, 5, 3, 9, 1, 6], [2, 1]]],
  ['bubble', bubbleSortSteps, [[5, 2, 9, 1, 7], [1, 2, 3], [3, 2, 1]]],
  ['quick', quickSortSteps, [[5, 2, 9, 1, 7, 3], [1], [2, 1], [4, 4, 4, 1]]]
]

describe.each(cases)('%s 生成器', (_name, fn, inputs) => {
  it.each(inputs.map(i => [i]))('输入 %j 最终帧有序', (input) => {
    const steps = fn(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
  })
  it('纯函数：不修改输入', () => {
    const input = [3, 1, 2]
    fn([...input])
    const steps = fn(input)
    expect(input).toEqual([3, 1, 2])
    expect(steps[0].state).toEqual([3, 1, 2]) // 首帧=初始状态
  })
})

describe('bubble 附加行为', () => {
  it('已有序输入触发提前结束（narration 含"提前结束"）', () => {
    const steps = bubbleSortSteps([1, 2, 3])
    expect(steps.some(s => s.narration.includes('提前结束'))).toBe(true)
  })
})
describe('quick 附加行为', () => {
  it('narration 提及 pivot 与区间', () => {
    const steps = quickSortSteps([5, 2, 9, 1])
    expect(steps.some(s => s.narration.includes('pivot'))).toBe(true)
  })
})
