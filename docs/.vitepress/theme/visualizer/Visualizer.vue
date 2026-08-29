<!-- docs/.vitepress/theme/visualizer/Visualizer.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import { registry } from './registry'
import PlayerShell from './PlayerShell.vue'
import ArrayBar from './renderers/ArrayBar.vue'

const props = defineProps<{ algorithm: string }>()
const def = computed(() => registry[props.algorithm])
</script>

<template>
  <div v-if="def">
    <PlayerShell :def="def" v-slot="{ current }">
      <ArrayBar :step="current" />
    </PlayerShell>
  </div>
  <p v-else style="color: var(--vp-c-danger-1)">未知动画：{{ props.algorithm }}（registry 中不存在）</p>
</template>
