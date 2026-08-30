import { defineConfig } from 'vitepress'
import { genSidebar } from './genSidebar'
import { fileURLToPath } from 'node:url'

export default defineConfig({
  lang: 'zh-CN',
  title: '数据结构 · C 语言工程实现',
  description: '65 个自包含 C 模块的数据结构教程站，严蔚敏/王道体系，配算法动画演示',
  srcExclude: ['**/superpowers/**'],
  build: {
    rollupOptions: {
      output: {
        // 动画基建（62 个步骤生成器 + 5 类渲染器）拆独立 chunk，跨动画页共享缓存，
        // 避免每页重复打包 200KB+ 的 registry 全量依赖
        manualChunks: { visualizer: [/visualizer/] }
      }
    }
  },
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
    outline: 'deep',
    socialLinks: [
      { icon: 'github', link: 'https://github.com/IsXiaoXiaoZhou/DataStructure-C' }
    ]
  }
})
