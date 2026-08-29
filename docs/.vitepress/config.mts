import { defineConfig } from 'vitepress'

export default defineConfig({
  lang: 'zh-CN',
  title: '数据结构 · C 语言工程实现',
  description: '65 个自包含 C 模块的数据结构教程站，严蔚敏/王道体系，配算法动画演示',
  srcExclude: ['**/superpowers/**'],
  themeConfig: {
    sidebar: [],
    search: { provider: 'local' },
    outline: 'deep'
  }
})
