import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(repoRoot, 'docs')

export function buildModulePage({ category, module: mod, headers, sources }) {
  const embed = files => files.map(f => `<<< @/../${category}/${mod}/${f}`).join('\n\n')
  return `---
outline: deep
---

# ${mod}

> 本页为骨架页，讲解内容待填充：删除各节 <!-- 待填 --> 占位并按模板写正文。

## 一句话定位

<!-- 待填：这是什么结构、解决什么问题（1~2 句） -->

## 核心概念

<!-- 待填：设计要点、示意图（如需要） -->

## 复杂度表

<!-- 待填：各操作时间/空间复杂度（可用 ComplexityTable 组件） -->

## 关键代码

### 接口

${embed(headers)}

### 实现

${embed(sources.filter(f => f !== 'main.c'))}

### 运行与测试（main.c 内置断言）

${embed(sources.filter(f => f === 'main.c'))}

## 动画演示

<!-- 该模块暂无动画；动画基建完成后按批次挂 <Visualizer algorithm="..." /> -->

## 易错点 / 考点

<!-- 待填：王道考研高频陷阱 -->

## 动手跑

\`\`\`bash
cd ${category}/${mod}
gcc -Wall -Wextra -std=c99 *.c -o demo.exe && ./demo.exe
\`\`\`
`
}

function main() {
  let created = 0, skipped = 0
  for (const category of fs.readdirSync(repoRoot).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, d)).isDirectory()).sort()) {
    fs.mkdirSync(path.join(docsDir, category), { recursive: true })
    const indexMd = path.join(docsDir, category, 'index.md')
    if (!fs.existsSync(indexMd)) fs.writeFileSync(indexMd, `# ${category}\n\n<!-- 分类概述待撰写（Task 5） -->\n`)
    for (const mod of fs.readdirSync(path.join(repoRoot, category)).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, category, d)).isDirectory()).sort()) {
      const files = fs.readdirSync(path.join(repoRoot, category, mod)).filter(f => f.endsWith('.c') || f.endsWith('.h'))
      const headers = files.filter(f => f.endsWith('.h'))
      const sources = files.filter(f => f.endsWith('.c'))
      if (!headers.length) { console.warn(`跳过（无 .h）：${category}/${mod}`); continue }
      const target = path.join(docsDir, category, `${mod}.md`)
      if (fs.existsSync(target)) { skipped++; continue } // 已存在（手写或旧骨架）绝不覆盖
      fs.writeFileSync(target, buildModulePage({ category, module: mod, headers, sources }))
      created++
    }
  }
  console.log(`生成 ${created} 页，跳过 ${skipped} 页（已存在）`)
}

if (process.argv[1] === fileURLToPath(import.meta.url)) main()
