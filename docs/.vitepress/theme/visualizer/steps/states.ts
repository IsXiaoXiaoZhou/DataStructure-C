// docs/.vitepress/theme/visualizer/steps/states.ts
// 各渲染器的 state 形状（完整契约见渲染器文件头 JSDoc）：
//   renderers/StackQueue.vue —— slots/pointers + 全局展平高亮
//   renderers/ListNode.vue   —— nodes/headId + node.highlight 三态
//   renderers/Tree.vue       —— nodes(keys/children)/rootId + 键位/边双通道高亮（批3）

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

// 批3：Tree 渲染器（统一树形态，二叉是多叉特例）
export interface TNode {
  id: number
  /** 键集：0 个=外部/虚拟结点（虚线框），1 个=二叉，多个=B 树结点 */
  keys: (number | string)[]
  /** 孩子 id 数组（左→右）；B 树约定孩子数 = 键数 + 1 */
  children: number[]
  /** 孩子 L/R 槽位（与 children 平行，仅二叉结点需要）：缺省按一般树均匀铺开 */
  sides?: ('L' | 'R')[]
  /** 红黑树结点填充色（缺省中性底） */
  color?: 'R' | 'B'
  /** 结点上方小标签（权值/编号/BF/线索说明） */
  nodeLabel?: string
  /** 连向双亲的边上的小标签（如 2i / 2i+1） */
  edgeLabel?: string
  /** keys 下标：绿色（已完成/线索/命中） */
  keyHighlight?: number[]
  /** keys 下标：橙色（当前比较/操作） */
  keyActive?: number[]
  /** children 下标：从本结点出发的第 i 条孩子边高亮 */
  edgeActive?: number[]
}

export interface TreeState {
  nodes: TNode[]
  rootId: number | null
  note?: string
}
