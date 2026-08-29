import fs from 'node:fs'
import path from 'node:path'

export interface SidebarItem {
  text: string
  link?: string
  items?: SidebarItem[]
}

export function genSidebar(docsDir: string): SidebarItem[] {
  const categories = fs.readdirSync(docsDir, { withFileTypes: true })
    .filter(e => /^\d{2}_/.test(e.name) && e.isDirectory())
    .map(e => e.name)
    .sort()
  return categories.map(cat => {
    const pages = fs.readdirSync(path.join(docsDir, cat), { withFileTypes: true })
      .filter(e => e.isFile() && e.name.endsWith('.md') && e.name !== 'index.md')
      .map(e => e.name.replace(/\.md$/, ''))
      .sort()
    return {
      text: cat.replace(/^(\d+)_/, '$1 '),
      link: `/${cat}/`,
      items: pages.map(p => ({ text: p, link: `/${cat}/${p}` }))
    }
  })
}
