// docs/.vitepress/theme/visualizer/steps/states.ts
// 批2 两种渲染器的 state 形状（完整契约见渲染器文件头 JSDoc）：
//   renderers/StackQueue.vue —— slots/pointers + 全局展平高亮
//   renderers/ListNode.vue   —— nodes/headId + node.highlight 三态

export interface SqPointer { name: string; index: number }

export interface SqSlot {
  label: string
  items: (number | string)[]
  capacity?: number
  kind?: 'stack' | 'queue' | 'shared' | 'ring'
  pointers?: SqPointer[]
}

export interface StackQueueState { slots: SqSlot[] }

export interface LnNode {
  id: number
  value: number | string
  next?: number | null
  prev?: number | null
  head?: boolean
  tail?: boolean
  ring?: boolean
  highlight?: 'none' | 'active' | 'done'
  label?: string
}

export interface ListNodeState {
  nodes: LnNode[]
  headId: number | null
  note?: string
  headLabel?: string
}
