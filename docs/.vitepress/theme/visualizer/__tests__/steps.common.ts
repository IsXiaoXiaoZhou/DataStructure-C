// docs/.vitepress/theme/visualizer/__tests__/steps.common.ts
import { expect } from 'vitest'
import type { Step } from '../types'

export function expectSortedSteps(steps: Step[], input: number[]) {
  const last = steps[steps.length - 1]
  expect(last.state).toEqual([...input].sort((a, b) => a - b))
  expect(last.narration).toContain('完成')
}

export function expectFrameInvariants(steps: Step[], len: number) {
  expect(steps.length).toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length).toBeGreaterThan(0)
    expect((s.state as number[]).length).toBe(len)          // 每帧数组长度不变
    for (const i of [...s.highlights, ...(s.active ?? [])]) {
      expect(i).toBeGreaterThanOrEqual(0)
      expect(i).toBeLessThan(len)                            // 高亮不越界
    }
  }
}
