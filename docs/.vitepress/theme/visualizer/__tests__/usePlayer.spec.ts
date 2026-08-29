// docs/.vitepress/theme/visualizer/__tests__/usePlayer.spec.ts
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { ref, computed } from 'vue'
import { usePlayer } from '../usePlayer'
import type { Step } from '../types'

const mk = (n: number): Step[] =>
  Array.from({ length: n }, (_, i) => ({ state: [i], highlights: [], active: null, narration: `第${i}步` }))

beforeEach(() => vi.useFakeTimers())
afterEach(() => vi.useRealTimers())

describe('usePlayer', () => {
  it('初始停在第一帧，current 暴露当前 Step', () => {
    const p = usePlayer(ref(mk(3)))
    expect(p.index.value).toBe(0)
    expect(p.current.value!.narration).toBe('第0步')
    expect(p.total.value).toBe(3)
  })
  it('next/prev/seek 边界钳制', () => {
    const p = usePlayer(ref(mk(3)))
    p.next(); p.next()
    expect(p.index.value).toBe(2)
    p.next() // 末帧再 next 不越界
    expect(p.index.value).toBe(2)
    p.prev()
    expect(p.index.value).toBe(1)
    p.seek(99); expect(p.index.value).toBe(2)
    p.seek(-5); expect(p.index.value).toBe(0)
  })
  it('play 按 speed 自动前进，到末帧自动停', () => {
    const p = usePlayer(ref(mk(3)))
    p.setSpeed(100)
    p.play()
    expect(p.playing.value).toBe(true)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(1)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(2)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(2)
    expect(p.playing.value).toBe(false)
  })
  it('末帧再 play 从头开始；toggle 可暂停', () => {
    const p = usePlayer(ref(mk(2)))
    p.seek(1); p.play()
    expect(p.index.value).toBe(0)
    p.toggle()
    expect(p.playing.value).toBe(false)
  })
  it('steps 变化（换输入重算）时 index 重置到 0', () => {
    const steps = ref(mk(3))
    const p = usePlayer(steps)
    p.seek(2)
    steps.value = mk(5)
    expect(p.index.value).toBe(0)
    expect(p.total.value).toBe(5)
  })
  it('setSpeed 立即生效（播放中不重置进度）', () => {
    const p = usePlayer(ref(mk(4)))
    p.setSpeed(200); p.play()
    vi.advanceTimersByTime(200)
    expect(p.index.value).toBe(1)
    p.setSpeed(50) // 播放中调速
    expect(p.index.value).toBe(1)
    vi.advanceTimersByTime(50)
    expect(p.index.value).toBe(2)
  })
})
