// docs/scripts/check-docs.mjs
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(repoRoot, 'docs')

// 递归收集 docs/ 下全部 md（排除 superpowers、.vitepress、node_modules）
function walkMdFiles(dir, out = []) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    if (e.isDirectory()) {
      if (['superpowers', '.vitepress', 'node_modules'].includes(e.name)) continue
      walkMdFiles(path.join(dir, e.name), out)
    } else if (e.isFile() && e.name.endsWith('.md')) {
      out.push(path.join(dir, e.name))
    }
  }
  return out
}

export function collectIssues(repoRoot, docsDir) {
  const issues = []
  for (const category of fs.readdirSync(repoRoot).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, d)).isDirectory()).sort()) {
    for (const mod of fs.readdirSync(path.join(repoRoot, category)).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(repoRoot, category, d)).isDirectory()).sort()) {
      const md = path.join(docsDir, category, `${mod}.md`)
      if (!fs.existsSync(md)) { issues.push(`源模块 ${category}/${mod} 缺少讲解页 docs/${category}/${mod}.md`); continue }
    }
  }
  // 全量嵌码扫描：docs/ 下全部 md 的 <<< 引用路径存在性（@/ 相对 docs/，否则相对 md 所在目录）
  for (const md of walkMdFiles(docsDir)) {
    const text = fs.readFileSync(md, 'utf8')
    for (const m of text.matchAll(/^<<<\s+(\S+)$/gm)) {
      const target = m[1].startsWith("@/") ? path.resolve(docsDir, m[1].slice(2)) : path.resolve(path.dirname(md), m[1])
      if (!fs.existsSync(target)) {
        const rel = path.relative(docsDir, md).split(path.sep).join('/')
        issues.push(`docs/${rel} 引用不存在的源文件: ${m[1]}`)
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
