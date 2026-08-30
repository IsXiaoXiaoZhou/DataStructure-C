import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(repoRoot, 'docs')

// 按首段子目录分组（保持出现顺序）；应用集合模块的接口节/编译命令按子算法目录分组
function groupBySubdir(files) {
  const groups = new Map()
  for (const f of files) {
    const dir = f.split('/')[0]
    if (!groups.has(dir)) groups.set(dir, [])
    groups.get(dir).push(f)
  }
  return [...groups.entries()]
}

export function buildModulePage({ category, module: mod, headers, sources }) {
  const embed = files => files.map(f => `<<< @/../${category}/${mod}/${f}`).join('\n\n')
  const isMain = f => f === 'main.c' || f.endsWith('/main.c')
  // 应用集合模块：无顶层统一接口，.h/.c 都在子算法子目录中（每个子算法 X.c/X.h/main.c）
  const nested = sources.some(s => s.includes('/'))
  const interfaceSection = nested
    ? groupBySubdir(headers).map(([dir, hs]) => `### ${dir}\n\n${embed(hs)}`).join('\n\n') // 按子目录分组嵌各自的 .h
    : `### 接口\n\n${embed(headers)}`
  const runCmd = nested
    ? groupBySubdir(sources).map(([dir]) => `gcc -Wall -Wextra -std=c99 ${dir}/*.c -o demo.exe && ./demo.exe`).join('\n')
    : 'gcc -Wall -Wextra -std=c99 *.c -o demo.exe && ./demo.exe'
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

<!-- 待填：各操作时间/空间复杂度，建议用 Markdown 表格呈现（操作 × 时间/空间复杂度） -->

## 关键代码

${interfaceSection}

### 实现

${embed(sources.filter(f => !isMain(f)))}

### 运行与测试（main.c 内置断言）

${embed(sources.filter(isMain))}

## 动画演示

<!-- 本模块暂未挂载动画（基建已就绪，后续批次接入） <Visualizer algorithm="..." /> -->

## 易错点 / 考点

<!-- 待填：王道考研高频陷阱 -->

## 动手跑

\`\`\`bash
cd ${category}/${mod}
${runCmd}
\`\`\`
`
}

function collectNested(dir) {
  // 应用集合模块：算法在子目录中（每个子算法 X.c/X.h/main.c）；递归收集 .h 与 .c
  const out = []
  const walk = d => {
    for (const e of fs.readdirSync(d, { withFileTypes: true }).sort((a, b) => (a.name < b.name ? -1 : 1))) {
      const p = path.join(d, e.name)
      if (e.isDirectory()) walk(p)
      else if (e.name.endsWith('.c') || e.name.endsWith('.h')) out.push(path.relative(dir, p).split(path.sep).join('/'))
    }
  }
  walk(dir)
  return out
}

function main() {
  let created = 0, skipped = 0
  for (const category of fs.readdirSync(repoRoot).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, d)).isDirectory()).sort()) {
    fs.mkdirSync(path.join(docsDir, category), { recursive: true })
    const indexMd = path.join(docsDir, category, 'index.md')
    if (!fs.existsSync(indexMd)) fs.writeFileSync(indexMd, `# ${category}\n\n<!-- 分类概述待撰写（Task 5） -->\n`)
    for (const mod of fs.readdirSync(path.join(repoRoot, category)).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, category, d)).isDirectory()).sort()) {
      const modDir = path.join(repoRoot, category, mod)
      const topFiles = fs.readdirSync(modDir).filter(f => f.endsWith('.c') || f.endsWith('.h'))
      // 应用集合模块顶层无 .c/.h：递归收集子目录中的 .h 与 .c（相对路径以 / 分隔）
      const files = topFiles.length ? topFiles : collectNested(modDir)
      const headers = files.filter(f => f.endsWith('.h'))
      const sources = files.filter(f => f.endsWith('.c'))
      const target = path.join(docsDir, category, `${mod}.md`)
      if (fs.existsSync(target)) { skipped++; continue } // 已存在（手写或旧骨架）绝不覆盖
      fs.writeFileSync(target, buildModulePage({ category, module: mod, headers, sources }))
      created++
    }
  }
  console.log(`生成 ${created} 页，跳过 ${skipped} 页（已存在）`)
}

if (process.argv[1] === fileURLToPath(import.meta.url)) main()
