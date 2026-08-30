// docs/.vitepress/theme/visualizer/steps/graphKit.ts
// 批4 图类生成器共用小工具：环状布局坐标与加权边输入形状。
// 布局契约（写在生成器侧）：顶点按编号沿圆周排布（0 号在正上方，顺时针），
// 坐标系 0~100、圆心 (50,52)、半径 34，保证结点圈与边文字都不出 viewBox。
import type { Step } from '../types'

export interface WEdge { u: number; v: number; w?: number }
export interface GraphBuildInput { edges: WEdge[]; n: number }

export const RING = { cx: 50, cy: 52, r: 34 }

export function ringCoords(n: number): { x: number; y: number }[] {
  return Array.from({ length: n }, (_, i) => {
    const ang = (-90 + (i * 360) / n) * (Math.PI / 180)
    return { x: +(RING.cx + RING.r * Math.cos(ang)).toFixed(2), y: +(RING.cy + RING.r * Math.sin(ang)).toFixed(2) }
  })
}

/** 生成器统一帧构造：state 只装 Graph 契约字段，highlights/active 不用（高亮走 state 内） */
export function gframe(
  nodes: { id: number; label: string; x: number; y: number; highlight?: boolean; active?: boolean }[],
  edges: { from: number; to: number; weight?: number; directed?: boolean; highlight?: boolean; active?: boolean; label?: string }[],
  aux: { text: string }[],
  narration: string,
  note?: string
): Step {
  return { state: { nodes: nodes.map(n => ({ ...n })), edges: edges.map(e => ({ ...e })), aux: aux.map(a => ({ text: a.text })), ...(note ? { note } : {}) }, highlights: [], active: null, narration }
}

/** 无向边规范化键（小端在前），用于判重 */
export const ukey = (u: number, v: number) => (u <= v ? `${u}-${v}` : `${v}-${u}`)
