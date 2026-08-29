<!-- docs/.vitepress/theme/visualizer/renderers/ArrayBar.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

const props = defineProps<{ step: Step | null }>()

interface View { array: number[]; buckets: number[][]; bucketLabels: string[] }

const view = computed<View | null>(() => {
  if (!props.step) return null
  const s = props.step.state
  if (Array.isArray(s)) return { array: s as number[], buckets: [], bucketLabels: [] }
  const o = s as any
  if (o && Array.isArray(o.array)) return { array: o.array, buckets: o.buckets ?? [], bucketLabels: o.bucketLabels ?? [] }
  return null
})

const maxVal = computed(() => Math.max(1, ...(view.value?.array ?? [1])))

function cls(i: number) {
  const s = props.step
  if (!s) return ''
  if (s.active && (s.active[0] === i || s.active[1] === i)) return 'bar active'
  if (s.highlights.includes(i)) return 'bar done'
  return 'bar'
}

const H_MIN = 16, H_MAX = 150
const h = (v: number) => H_MIN + Math.round((v / maxVal.value) * (H_MAX - H_MIN))
</script>

<template>
  <div v-if="view" class="arraybar">
    <div class="row">
      <div v-for="(v, i) in view.array" :key="i" :class="cls(i)">
        <div class="fill" :style="{ height: h(v) + 'px' }"></div>
        <span class="val">{{ v }}</span>
        <span class="idx">{{ i }}</span>
      </div>
    </div>
    <div v-if="view.buckets.length" class="buckets">
      <div v-for="(b, bi) in view.buckets" :key="bi" class="bucket">
        <div class="bucket-label">{{ view.bucketLabels[bi] }}</div>
        <div class="bucket-items"><span v-for="(x, xi) in b" :key="xi">{{ x }}</span></div>
      </div>
    </div>
    <div class="legend">
      <span class="chip"><i class="swatch done"></i>已就位</span>
      <span class="chip"><i class="swatch active"></i>当前比较/交换</span>
      <span class="chip"><i class="swatch"></i>未处理</span>
    </div>
  </div>
</template>

<style scoped>
.arraybar { padding: 12px 0; }
.row { display: flex; align-items: flex-end; gap: 6px; min-height: 190px; flex-wrap: wrap; }
.bar { display: flex; flex-direction: column; justify-content: flex-end; align-items: center; width: 34px; border-radius: 4px 4px 0 0; }
.fill { width: 100%; background: var(--vp-c-indigo-2); border-radius: 3px 3px 0 0; transition: height .25s, background .25s; }
.bar.done .fill { background: var(--vp-c-green-2); }
.bar.active .fill { background: var(--vp-c-yellow-2); }
.val { font-size: 12px; margin-top: 2px; font-variant-numeric: tabular-nums; }
.idx { font-size: 10px; opacity: .55; }
.buckets { display: flex; gap: 6px; margin-top: 14px; flex-wrap: wrap; }
.bucket { border: 1px solid var(--vp-c-divider); border-radius: 6px; min-width: 40px; padding: 4px 6px; text-align: center; }
.bucket-label { font-size: 11px; opacity: .6; }
.bucket-items { display: flex; gap: 4px; justify-content: center; min-height: 20px; font-size: 12px; }
.legend { display: flex; gap: 14px; margin-top: 12px; font-size: 12px; opacity: .8; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.done { background: var(--vp-c-green-2); border: none; }
.swatch.active { background: var(--vp-c-yellow-2); border: none; }
</style>
