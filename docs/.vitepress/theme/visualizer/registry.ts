// docs/.vitepress/theme/visualizer/registry.ts
import type { VisualizerDef } from './types'
import { insertionSortSteps } from './steps/insertionSort'
import { shellSortSteps } from './steps/shellSort'
import { bubbleSortSteps } from './steps/bubbleSort'
import { quickSortSteps } from './steps/quickSort'
import { selectionSortSteps } from './steps/selectionSort'
import { heapSortSteps } from './steps/heapSort'
import { mergeSortSteps } from './steps/mergeSort'
import { countingSortSteps } from './steps/countingSort'
import { radixSortSteps } from './steps/radixSort'

function numberList(min: number, max: number, maxLen: number) {
  return {
    parse: (text: string) => text.split(/[,，\s]+/).filter(s => s.length).map(Number),
    validate: (input: unknown): string | null => {
      if (!Array.isArray(input) || input.length === 0) return '至少输入 1 个数字'
      if (input.length > maxLen) return `最多 ${maxLen} 个元素`
      if (input.some(v => !Number.isInteger(v) || v < min || v > max)) return `值需为 ${min}~${max} 的整数`
      return null
    }
  }
}

const sortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 999, 20)
})
const bucketSortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 99, 20)
})

export const registry: Record<string, VisualizerDef> = {
  'insertion-sort': sortDef('插入排序', insertionSortSteps, '5,2,9,1,7'),
  'shell-sort': sortDef('希尔排序', shellSortSteps, '8,5,3,9,1,6'),
  'bubble-sort': sortDef('冒泡排序', bubbleSortSteps, '5,2,9,1,7'),
  'quick-sort': sortDef('快速排序', quickSortSteps, '5,2,9,1,7,3'),
  'selection-sort': sortDef('简单选择排序', selectionSortSteps, '5,2,9,1,7'),
  'heap-sort': sortDef('堆排序', heapSortSteps, '5,2,9,1,7,3'),
  'merge-sort': sortDef('归并排序', mergeSortSteps, '5,2,9,1,7,3'),
  'counting-sort': bucketSortDef('计数排序（值域 0~9）', countingSortSteps, '3,1,4,1,5'),
  'radix-sort': bucketSortDef('基数排序（LSD，值域 0~99）', radixSortSteps, '42,7,91,7,30')
}
