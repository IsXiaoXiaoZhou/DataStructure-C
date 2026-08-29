<!-- docs/.vitepress/theme/visualizer/PlayerShell.vue -->
<script setup lang="ts">
import { ref, computed } from 'vue'
import { usePlayer } from './usePlayer'
import type { VisualizerDef } from './types'

const props = defineProps<{ def: VisualizerDef }>()

const inputText = ref(props.def.defaultInput)
const error = ref<string | null>(null)

const parsed = computed(() => {
  error.value = null
  try {
    const input = props.def.parse(inputText.value)
    const err = props.def.validate(input)
    if (err) { error.value = err; return null }
    return input
  } catch {
    error.value = '输入格式不正确'
    return null
  }
})

const steps = computed(() => (parsed.value === null ? [] : props.def.steps(parsed.value)))
const player = usePlayer(steps)
const { index, playing, speed, current, total } = player

const speedOptions = [ { label: '0.5×', ms: 800 }, { label: '1×', ms: 400 }, { label: '2×', ms: 200 }, { label: '4×', ms: 100 } ]
</script>

<template>
  <div class="player">
    <div class="title">{{ def.title }}</div>
    <div class="stage"><slot :current="current" /></div>
    <div v-if="current" class="narration">{{ current.narration }}</div>
    <div v-if="error" class="err">⚠ {{ error }}（示例输入：{{ def.defaultInput }}）</div>
    <div class="controls">
      <button @click="player.prev()" :disabled="index === 0">⏮ 上一步</button>
      <button class="primary" @click="player.toggle()" :disabled="!total"> {{ playing ? '⏸ 暂停' : '▶ 播放' }} </button>
      <button @click="player.next()" :disabled="index >= total - 1">下一步 ⏭</button>
      <input class="range" type="range" min="0" :max="Math.max(0, total - 1)" :value="index"
             @input="player.seek(Number(($event.target as HTMLInputElement).value))" :disabled="!total" />
      <span class="counter">{{ total ? index + 1 : 0 }} / {{ total }}</span>
      <select :value="String(speed)" @change="player.setSpeed(Number(($event.target as HTMLSelectElement).value))">
        <option v-for="o in speedOptions" :key="o.ms" :value="String(o.ms)">{{ o.label }}</option>
      </select>
    </div>
    <div class="input-row">
      <label>输入数据：</label>
      <input v-model="inputText" placeholder="如 5,2,9,1,7" />
    </div>
  </div>
</template>

<style scoped>
.player { border: 1px solid var(--vp-c-divider); border-radius: 10px; padding: 14px 16px; margin: 12px 0; background: var(--vp-c-bg-soft); }
.title { font-weight: 600; margin-bottom: 8px; }
.narration { margin-top: 10px; padding: 8px 10px; border-radius: 6px; background: var(--vp-c-bg); font-size: 14px; min-height: 20px; }
.err { color: var(--vp-c-danger-1); font-size: 13px; margin-top: 6px; }
.controls { display: flex; align-items: center; gap: 8px; margin-top: 10px; flex-wrap: wrap; }
.controls button { padding: 4px 10px; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg); cursor: pointer; }
.controls button.primary { background: var(--vp-c-brand-1); color: #fff; border: none; }
.controls button:disabled { opacity: .45; cursor: not-allowed; }
.range { flex: 1; min-width: 120px; }
.counter { font-size: 12px; opacity: .7; font-variant-numeric: tabular-nums; }
.input-row { display: flex; align-items: center; gap: 8px; margin-top: 10px; font-size: 13px; }
.input-row input { flex: 1; padding: 4px 8px; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg); color: var(--vp-c-text-1); }
</style>
