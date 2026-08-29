// docs/.vitepress/__tests__/genSidebar.spec.ts
import { describe, it, expect, beforeAll, afterAll } from 'vitest'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import { genSidebar } from '../genSidebar'

let tmp: string
beforeAll(() => {
  tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'sb-'))
  for (const cat of ['01_线性表', '09_排序']) {
    fs.mkdirSync(path.join(tmp, cat), { recursive: true })
    fs.writeFileSync(path.join(tmp, cat, 'index.md'), '')
  }
  fs.writeFileSync(path.join(tmp, '01_线性表', '01_静态顺序表.md'), '')
  fs.writeFileSync(path.join(tmp, '01_线性表', '03_单链表.md'), '')
  fs.writeFileSync(path.join(tmp, '09_排序', '04_快速排序.md'), '')
})
afterAll(() => fs.rmSync(tmp, { recursive: true, force: true }))

describe('genSidebar', () => {
  it('按 0X_ 目录生成分组，模块页排序且去掉 .md', () => {
    const sb = genSidebar(tmp)
    expect(sb).toHaveLength(2)
    expect(sb[0].text).toBe('01 线性表')
    expect(sb[0].link).toBe('/01_线性表/')
    expect(sb[0].items!.map(i => i.text)).toEqual(['01_静态顺序表', '03_单链表'])
    expect(sb[0].items![0].link).toBe('/01_线性表/01_静态顺序表')
  })
  it('不含 index.md 自身', () => {
    const sb = genSidebar(tmp)
    expect(sb[0].items!.some(i => i.text === 'index')).toBe(false)
  })
  it('未来新增分类目录自动收录', () => {
    fs.mkdirSync(path.join(tmp, '10_新分类'), { recursive: true })
    fs.writeFileSync(path.join(tmp, '10_新分类', '01_新模块.md'), '')
    const sb = genSidebar(tmp)
    expect(sb).toHaveLength(3)
    expect(sb[2].text).toBe('10 新分类')
  })
})
