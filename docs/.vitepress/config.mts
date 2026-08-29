import { defineConfig } from 'vitepress'
import { genSidebar } from './genSidebar'
import { fileURLToPath } from 'node:url'

export default defineConfig({
  lang: 'zh-CN',
  title: '数据结构 · C 语言工程实现',
  description: '65 个自包含 C 模块的数据结构教程站，严蔚敏/王道体系，配算法动画演示',
  srcExclude: ['**/superpowers/**'],
  markdown: {
    languageAlias: { h: 'c' }
  },
  themeConfig: {
    nav: [
      { text: '首页', link: '/' },
      { text: '学习路线', link: '/guide/learning-path' },
      { text: '工程规范', link: '/guide/conventions' },
      { text: '动画示例', link: '/guide/visualizer-demo' }
    ],
    sidebar: genSidebar(fileURLToPath(new URL('../', import.meta.url))),
    search: { provider: 'local' },
    outline: 'deep'
  }
})
