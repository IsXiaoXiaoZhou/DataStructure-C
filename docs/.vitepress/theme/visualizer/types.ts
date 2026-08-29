// docs/.vitepress/theme/visualizer/types.ts
export interface Step {
  state: unknown
  highlights: number[]
  active: [number, number] | null
  narration: string
}

export interface VisualizerDef {
  title: string
  renderer: 'arrayBar'
  parse: (text: string) => unknown
  validate: (input: unknown) => string | null
  steps: (input: any) => Step[]
  defaultInput: string
}
