import { describe, it, expect } from 'vitest'
import { buildModulePage } from '../gen-module-pages.mjs'

const mod = {
  category: '01_线性表',
  module: '01_静态顺序表',
  headers: ['static_seq_list.h'],
  sources: ['static_seq_list.c', 'main.c']
}

describe('buildModulePage', () => {
  it('frontmatter 与标题', () => {
    const md = buildModulePage(mod)
    expect(md).toMatch(/^---\noutline: deep\n---\n\n# 01_静态顺序表\n/)
  })
  it('嵌码指令相对路径指向仓库源文件', () => {
    const md = buildModulePage(mod)
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/static_seq_list.h')
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/static_seq_list.c')
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/main.c')
  })
  it('含六大模板节与占位提示', () => {
    const md = buildModulePage(mod)
    for (const sec of ['## 一句话定位', '## 核心概念', '## 复杂度表', '## 关键代码', '## 易错点 / 考点', '## 动手跑'])
      expect(md).toContain(sec)
    expect(md).toContain('<!-- 待填 -->')
  })
  it('动手跑给出该分类的 cd 路径', () => {
    expect(buildModulePage(mod)).toContain('cd 01_线性表/01_静态顺序表')
  })
})
