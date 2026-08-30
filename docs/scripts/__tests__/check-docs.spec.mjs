// docs/scripts/__tests__/check-docs.spec.mjs
import { describe, it, expect, beforeAll, afterAll } from 'vitest'
import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'
import { collectIssues } from '../check-docs.mjs'

let repo, docs
beforeAll(() => {
  repo = fs.mkdtempSync(path.join(os.tmpdir(), 'chk-'))
  docs = path.join(repo, 'docs')
  // 源模块 A：正常
  fs.mkdirSync(path.join(repo, '01_线性表', '01_静态顺序表'), { recursive: true })
  fs.writeFileSync(path.join(repo, '01_线性表', '01_静态顺序表', 'a.h'), '')
  // 源模块 B：缺 md
  fs.mkdirSync(path.join(repo, '01_线性表', '02_动态顺序表'), { recursive: true })
  fs.writeFileSync(path.join(repo, '01_线性表', '02_动态顺序表', 'b.h'), '')
  // docs：A 的 md 引用存在文件 + 不存在文件
  fs.mkdirSync(path.join(docs, '01_线性表'), { recursive: true })
  fs.writeFileSync(path.join(docs, '01_线性表', '01_静态顺序表.md'),
    '<<< @/../01_线性表/01_静态顺序表/a.h\n<<< @/../01_线性表/01_静态顺序表/不存在.c\n')
})
afterAll(() => { fs.rmSync(repo, { recursive: true, force: true }) })

describe('collectIssues', () => {
  it('报告缺 md 的源模块', () => {
    const issues = collectIssues(repo, docs)
    expect(issues.some(s => s.includes('02_动态顺序表') && s.includes('缺少'))).toBe(true)
  })
  it('报告 <<< 指向不存在的源文件', () => {
    const issues = collectIssues(repo, docs)
    expect(issues.some(s => s.includes('不存在.c'))).toBe(true)
  })
  it('正常引用与正常模块不产生问题', () => {
    const issues = collectIssues(repo, docs)
    expect(issues.some(s => s.includes('a.h'))).toBe(false)
    expect(issues.some(s => s.includes('01_静态顺序表') && s.includes('缺少'))).toBe(false)
  })
  it('docs/ 下非模块页（如 guide）的嵌码也纳入扫描', () => {
    // 独立夹具：guide 页扫描不依赖模块目录，且不污染上方共享的 repo/docs 夹具
    let repo2
    try {
      repo2 = fs.mkdtempSync(path.join(os.tmpdir(), 'chk-guide-'))
      const docs2 = path.join(repo2, 'docs')
      fs.mkdirSync(path.join(docs2, 'guide'), { recursive: true })
      fs.writeFileSync(path.join(docs2, 'guide', 'snippet.txt'), 'demo')
      fs.writeFileSync(path.join(docs2, 'guide', '某.md'), '<<< ./snippet.txt\n<<< ./不存在.txt\n')
      const issues = collectIssues(repo2, docs2)
      expect(issues.some(s => s.includes('guide/某.md') && s.includes('不存在.txt'))).toBe(true)
      expect(issues.some(s => s.includes('snippet.txt'))).toBe(false)
    } finally {
      if (repo2) fs.rmSync(repo2, { recursive: true, force: true })
    }
  })
})
