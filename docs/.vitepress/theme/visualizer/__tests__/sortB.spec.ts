// docs/.vitepress/theme/visualizer/__tests__/sortB.spec.ts
import { describe, it, expect } from 'vitest'
import { selectionSortSteps } from '../steps/selectionSort'
import { heapSortSteps } from '../steps/heapSort'
import { mergeSortSteps } from '../steps/mergeSort'
import { countingSortSteps } from '../steps/countingSort'
import { radixSortSteps } from '../steps/radixSort'
import { expectSortedSteps, expectFrameInvariants } from './steps.common'

describe('selection', () => {
  it.each([[[5, 2, 9, 1, 7]], [[3, 3, 1]], [[1]]])('%j 最终帧有序', (input) => {
    const steps = selectionSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
    expect(steps.some(s => s.narration.includes('最小'))).toBe(true)
  })
})

describe('heap', () => {
  it.each([[[5, 2, 9, 1, 7, 3]], [[1]], [[2, 1]]])('%j 最终帧有序', (input) => {
    const steps = heapSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
    expect(steps.some(s => s.narration.includes('建堆'))).toBe(true)
  })
})

describe('merge', () => {
  it.each([[[5, 2, 9, 1, 7, 3]], [[1]], [[2, 1]]])('%j 最终帧有序', (input) => {
    const steps = mergeSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
  })
})

describe('counting（值域 0~9，桶形态）', () => {
  it('末帧 array 有序', () => {
    const steps = countingSortSteps([3, 1, 4, 1, 5])
    const last = steps[steps.length - 1].state as any
    expect(last.array).toEqual([1, 1, 3, 4, 5])
    expect(last.bucketLabels).toEqual(['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'])
  })
  it('纯函数：不修改输入', () => {
    const input = [3, 1, 2]
    countingSortSteps(input)
    expect(input).toEqual([3, 1, 2])
  })
  it('收集帧 array 恒为输入的置换（[9,0] 与 [3,1,4,1,5]）', () => {
    const sorted = (xs: number[]) => [...xs].sort((m, n) => m - n)
    for (const input of [[9, 0], [3, 1, 4, 1, 5]] as number[][]) {
      const steps = countingSortSteps(input)
      const target = sorted(input)
      for (const s of steps.filter(s => s.narration.includes('收集 →'))) {
        expect(sorted((s.state as any).array)).toEqual(target)
      }
      expect(sorted((steps[steps.length - 1].state as any).array)).toEqual(target)
    }
  })
})

describe('radix（值域 0~99，LSD 十位分桶）', () => {
  it('末帧 array 有序', () => {
    const steps = radixSortSteps([42, 7, 91, 7, 30])
    const last = steps[steps.length - 1].state as any
    expect(last.array).toEqual([7, 7, 30, 42, 91])
  })
  it('出现个位轮与十位轮两轮分发', () => {
    const steps = radixSortSteps([42, 7, 91])
    expect(steps.some(s => s.narration.includes('个位'))).toBe(true)
    expect(steps.some(s => s.narration.includes('十位'))).toBe(true)
  })
})
