// docs/scripts/check-docs.mjs
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(repoRoot, 'docs')

export function collectIssues(repoRoot, docsDir) {
  const issues = []
  for (const category of fs.readdirSync(repoRoot).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, d)).isDirectory()).sort()) {
    for (const mod of fs.readdirSync(path.join(repoRoot, category)).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, category, d)).isDirectory()).sort()) {
      const md = path.join(docsDir, category, `${mod}.md`)
      if (!fs.existsSync(md)) { issues.push(`源模块 ${category}/${mod} 缺少讲解页 docs/${category}/${mod}.md`); continue }
      const text = fs.readFileSync(md, 'utf8')
      for (const m of text.matchAll(/^<<<\s+(\S+)$/gm)) {
        const target = m[1].startsWith("@/") ? path.resolve(docsDir, m[1].slice(2)) : path.resolve(path.dirname(md), m[1])
        if (!fs.existsSync(target)) issues.push(`${category}/${mod}.md 引用不存在的源文件: ${m[1]}`)
      }
    }
  }
  return issues
}

function main() {
  const issues = collectIssues(repoRoot, docsDir)
  if (issues.length) { issues.forEach(s => console.error('✗ ' + s)); process.exit(1) }
  console.log('✓ 文档完整性检查通过')
}

if (process.argv[1] === fileURLToPath(import.meta.url)) main()
