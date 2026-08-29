// docs/.vitepress/theme/visualizer/usePlayer.ts
import { ref, computed, watch, onUnmounted, getCurrentInstance } from 'vue'
import type { Ref, ComputedRef } from 'vue'
import type { Step } from './types'

export function usePlayer(steps: Ref<Step[]>) {
  const index = ref(0)
  const playing = ref(false)
  const speed = ref(400)
  let timer: ReturnType<typeof setInterval> | null = null

  const total: ComputedRef<number> = computed(() => steps.value.length)
  const current = computed(() => steps.value[index.value] ?? null)

  function stopTimer() { if (timer) { clearInterval(timer); timer = null } }
  function pause() { playing.value = false; stopTimer() }
  function play() {
    if (index.value >= steps.value.length - 1) index.value = 0
    playing.value = true
    stopTimer()
    timer = setInterval(() => {
      if (index.value < steps.value.length - 1) index.value++
      else pause()
    }, speed.value)
  }
  function toggle() { playing.value ? pause() : play() }
  function next() { if (index.value < steps.value.length - 1) index.value++; else pause() }
  function prev() { if (index.value > 0) index.value-- }
  function seek(i: number) { index.value = steps.value.length === 0 ? 0 : Math.min(Math.max(0, i), steps.value.length - 1) }
  function setSpeed(ms: number) {
    speed.value = ms
    if (playing.value) { stopTimer(); timer = setInterval(() => {
      if (index.value < steps.value.length - 1) index.value++; else pause()
    }, speed.value) }
  }

  watch(steps, () => { index.value = 0 }, { flush: 'sync' }) // 同步重置：换输入重算后 index 立即归零，避免 current 短暂越界
  if (getCurrentInstance()) onUnmounted(stopTimer) // 测试环境无组件实例时不挂

  return { index, playing, speed, current, total, play, pause, toggle, next, prev, seek, setSpeed }
}
