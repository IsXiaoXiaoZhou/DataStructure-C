// docs/.vitepress/theme/visualizer/types.ts
export interface Step {
  state: unknown
  highlights: number[]
  /** 单元素=单指针高亮，双元素=比较/交换对 */
  active: number[] | null
  narration: string
}

export type RendererKind = 'arrayBar' | 'stackQueue' | 'listNode' | 'tree'

export interface VisualizerDef {
  title: string
  renderer: RendererKind
  parse: (text: string) => unknown
  validate: (input: unknown) => string | null
  steps: (input: any) => Step[]
  defaultInput: string
}
