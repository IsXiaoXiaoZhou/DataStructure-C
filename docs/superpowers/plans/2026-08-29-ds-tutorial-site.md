# 数据结构教程站点（首期）实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 65 个 C 数据结构模块建立 VitePress 教程站：站点基建 + 65 骨架页 + 动画基建 + V1 批次（9 种排序动画）。

**Architecture:** 现有 C 代码目录零改动，站点挂 `docs/`；讲解手写 Markdown，源码用 VitePress 原生 `<<<` 语法构建时自动嵌入；动画走"录像-回放"——步骤生成器纯函数（Vitest 单测）→ 通用播放器 → 按形态复用的渲染器。

**Tech Stack:** VitePress 1.x + Vue 3 + Vitest（全部 devDependencies，共 4 个）

**Spec:** `docs/superpowers/specs/2026-08-29-ds-tutorial-site-design.md`（计划从 spec 出发，执行者两者都读）

## Global Constraints

- 环境约定：Windows + Git Bash；Node ≥ 18（本机 v24.18.0）、npm 11.16.0
- 依赖上限：仅 `vitepress`、`vue`、`vitest` 三个包 + vue 的 peer，不引入 `@vue/test-utils` 等其他运行/测试依赖（spec 明确）
- **现有 `01_线性表/`~`09_排序/` 的 C 源码目录零改动**（任何任务都不得修改/移动/删除它们）
- 嵌码一律优先用 VitePress 原生 `<<< ../分类/模块/文件.c` 语法（相对 md 文件）
- 动画输入上限：排序类长度 1~20、值 0~999；计数/基数排序值 0~99；非法输入即时校验提示（spec 错误处理节）
- 站点全中文文案；界面语言 `lang: 'zh-CN'`
- 每个任务结束单独 commit（feat/test/docs 前缀），不与他人改动混提
- 所有新文件放 `docs/` 下（spec：站点相关文件全部收在 docs/，含 `package.json`）；**仓库根零新增文件**，仅修改 `.gitignore`
- npm/npx/vitest 命令一律在 `docs/` 目录下执行（下文命令已带 `cd docs &&` 前缀）；git 命令在仓库根执行

---

### Task 1: 工程基建——package.json、依赖、.gitignore

**Files:**
- Create: `docs/package.json`
- Modify: `.gitignore`

**Interfaces:**
- Produces: npm scripts `docs:dev` / `docs:build` / `docs:preview` / `check` / `gen:pages` / `test`（后续所有任务的验证入口；npm run 的 cwd 是 `docs/`，故 vitepress 的 srcDir 就是当前目录，脚本路径写 `scripts/...`）

- [ ] **Step 1: 写 docs/package.json**

```json
{
  "name": "datastructure-site",
  "private": true,
  "type": "module",
  "scripts": {
    "docs:dev": "vitepress dev",
    "docs:build": "vitepress build",
    "docs:preview": "vitepress preview",
    "gen:pages": "node scripts/gen-module-pages.mjs",
    "check": "node scripts/check-docs.mjs",
    "test": "vitest run"
  }
}
```

- [ ] **Step 2: 安装依赖**

```bash
cd docs && npm install -D vitepress vue vitest
```

Expected: 安装成功，`docs/node_modules/` 出现。

- [ ] **Step 3: 更新 .gitignore**

在现有内容基础上追加（保留现有行，注意当前工作区有一处未提交的 .gitignore 改动，一并保留）：

```gitignore
node_modules/
docs/.vitepress/cache/
docs/.vitepress/dist/
```

- [ ] **Step 4: 验证**

Run: `cd docs && npx vitepress --version && node -e "console.log(Object.keys(require('./package.json').scripts))"`
Expected: 版本号 1.x；scripts 六条齐全。

- [ ] **Step 5: Commit**

```bash
git add docs/package.json docs/package-lock.json .gitignore
git commit -m "chore: 站点工程基建（VitePress + Vitest 依赖与脚本）"
```

---

### Task 2: VitePress 最小站点 + `<<<` 跨目录嵌码验证 + 首页

**Files:**
- Create: `docs/.vitepress/config.mts`
- Create: `docs/index.md`
- Create（临时，验证后删除）: `docs/01_线性表/__sandbox__.md`

**Interfaces:**
- Produces: 可构建的站点骨架；config.mts 中 sidebar 暂为空数组（Task 3 接入 genSidebar）
- **决定关键风险**：`<<<` 能否引用 srcDir（docs/）之外的仓库源文件。能 → 全项目沿用；不能 → 在本任务内改用回退方案（见 Step 3b）

- [ ] **Step 1: 最小配置 config.mts**

```ts
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
```

（nav、socialLinks 在 Task 5/12 补全，此处保持最小以便隔离验证。）

- [ ] **Step 2: 首页 index.md**

```md
---
layout: home

hero:
  name: "数据结构"
  text: "C 语言工程实现"
  tagline: 65 个自包含模块 · 严蔚敏/王道体系 · 断言测试对拍验证 · 算法动画演示
  actions:
    - theme: brand
      text: 学习路线
      link: /guide/learning-path
    - theme: alt
      text: 工程规范
      link: /guide/conventions

features:
  - icon: 📋
    title: 01 线性表
    details: 顺序表 / 单双循环静态链表——一切"顺序与列表"数据的骨架
    link: /01_线性表/
  - icon: 🥞
    title: 02 栈
    details: 顺序栈 / 共享栈 / 链栈与括号匹配、表达式求值等经典应用
    link: /02_栈/
  - icon: 🚶
    title: 03 队列
    details: 循环队列 / 链队列 / 舞伴问题——先进先出的排队艺术
    link: /03_队列/
  - icon: 🧮
    title: 04 特殊矩阵压缩存储
    details: 对称 / 三角 / 对角 / 稀疏矩阵三元组与十字链表
    link: /04_特殊矩阵压缩存储/
  - icon: 🔤
    title: 05 串
    details: 定长 / 堆分配 / 块链串与朴素匹配、KMP(next+nextval)
    link: /05_串/
  - icon: 🌳
    title: 06 树与二叉树
    details: 三种存储 / 遍历 / 线索化 / 哈夫曼 / 并查集
    link: /06_树/
  - icon: 🕸️
    title: 07 图
    details: 四种存储 / DFS·BFS / MST / 最短路 / 拓扑 / 关键路径
    link: /07_图/
  - icon: 🔍
    title: 08 查找
    details: 折半 / BST / AVL / 红黑 / B 树 / 散列——从海量数据快速定位
    link: /08_查找/
  - icon: 🔀
    title: 09 排序
    details: 插入 / 交换 / 选择 / 归并 / 计数基数 / 外排，全程可动画演示
    link: /09_排序/
---
```

- [ ] **Step 3: 验证 `<<<` 跨目录嵌码（关键风险点）**

创建 `docs/01_线性表/__sandbox__.md`：

```md
# 嵌码试验

<<< ../01_线性表/01_静态顺序表/static_seq_list.h
```

Run: `cd docs && npm run docs:build`
Expected: 构建成功。打开 `docs/.vitepress/dist/01_线性表/__sandbox__.html` 确认含 C 代码高亮内容。

- [ ] **Step 3b（仅当 Step 3 失败）: 回退方案——markdown-it 自定义指令**

若 build 报"文件不存在"或输出不含代码：在 config.mts 增加自实现指令（保持 Markdown 侧语法 `<<<` 不变，用 config.mts 的 markdown.config.postParse 无法实现——改为 `markdown-it` 的 `core` 规则读 md 源文本，匹配 `<<< ../...` 行，在 `transform` 阶段替换为读入文件的 fenced code block）：

```ts
import fs from 'node:fs'
import path from 'node:path'

function codeInclude(md: any) {
  md.core.ruler.after('block', 'code-include', (state: any) => {
    const dir = path.dirname(state.env?.relativePath ? path.join(process.cwd(), 'docs', state.env.relativePath) : '.')
    state.tokens.forEach((tok: any) => { /* 逐行扫描 inline token，替换 <<< 行 */ })
  })
}
```

> 实现以"构建时读源文件、渲染为 fenced code block"为准绳；具体 token 遍历写法以 markdown-it 1.x API 为准（`state.tokens` 中 `type === 'inline'`，修改其 `content` 并重建子 token）。若走到 3b，先在 sandbox 页用最小 token 操作打通再删 sandbox。

- [ ] **Step 4: 删除试验页并复验**

```bash
rm docs/01_线性表/__sandbox__.md
cd docs && npm run docs:build
```

Expected: 构建成功且无残留页面。记录结论（Step 3 成功 / 走 3b）到 commit message。

- [ ] **Step 5: Commit**

```bash
git add docs/.vitepress/config.mts docs/index.md
git commit -m "feat: VitePress 站点骨架与首页；验证 <<< 跨目录嵌码"
```

---

### Task 3: genSidebar 侧边栏自动生成（TDD）

**Files:**
- Create: `docs/.vitepress/genSidebar.ts`
- Create: `docs/.vitepress/__tests__/genSidebar.spec.ts`
- Modify: `docs/.vitepress/config.mts`（接入 sidebar 与 nav）

**Interfaces:**
- Produces: `genSidebar(docsDir?: string): SidebarItem[]`，其中 `SidebarItem = { text: string; link?: string; items?: SidebarItem[] }`。默认扫描 docsDir 下所有匹配 `/^\d{2}_/` 的一级目录（自动收录未来新分类，spec"错误处理与边界"节）

- [ ] **Step 1: 写失败测试**

```ts
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
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run .vitepress/__tests__/genSidebar.spec.ts`
Expected: FAIL（genSidebar 模块不存在）。

- [ ] **Step 3: 实现 genSidebar.ts**

```ts
import fs from 'node:fs'
import path from 'node:path'

export interface SidebarItem {
  text: string
  link?: string
  items?: SidebarItem[]
}

export function genSidebar(docsDir: string): SidebarItem[] {
  const categories = fs.readdirSync(docsDir)
    .filter(name => /^\d{2}_/.test(name) && fs.statSync(path.join(docsDir, name)).isDirectory())
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
```

- [ ] **Step 4: 跑测试确认通过**

Run: `cd docs && npx vitest run .vitepress/__tests__/genSidebar.spec.ts`
Expected: PASS ×3。

- [ ] **Step 5: 接入 config.mts 并构建验证**

config.mts 修改为：

```ts
import { defineConfig } from 'vitepress'
import { genSidebar } from './genSidebar'
import { fileURLToPath } from 'node:url'

export default defineConfig({
  lang: 'zh-CN',
  title: '数据结构 · C 语言工程实现',
  description: '65 个自包含 C 模块的数据结构教程站，严蔚敏/王道体系，配算法动画演示',
  srcExclude: ['**/superpowers/**'],
  themeConfig: {
    nav: [
      { text: '首页', link: '/' },
      { text: '学习路线', link: '/guide/learning-path' },
      { text: '工程规范', link: '/guide/conventions' }
    ],
    sidebar: genSidebar(fileURLToPath(new URL('../', import.meta.url))),
    search: { provider: 'local' },
    outline: 'deep'
  }
})
```

Run: `cd docs && npm run docs:build`
Expected: 构建成功（此时分类页还不存在，链接由 Task 5 补齐；build 不做未引用链接检查，能过即可）。

- [ ] **Step 6: Commit**

```bash
git add docs/.vitepress/genSidebar.ts docs/.vitepress/__tests__/genSidebar.spec.ts docs/.vitepress/config.mts
git commit -m "feat: 侧边栏自动生成（扫描分类目录，新分类自动收录）"
```

---

### Task 4: 骨架页生成脚本（TDD）+ 全量生成 65 页

**Files:**
- Create: `docs/scripts/gen-module-pages.mjs`
- Create: `docs/scripts/__tests__/gen-module-pages.spec.mjs`
- Create（生成产物）: `docs/01_线性表/01_静态顺序表.md` … 共 65 个模块页

**Interfaces:**
- Consumes: 仓库现有 `0X_分类/0Y_模块/{*.h,*.c,main.c}` 目录结构
- Produces: `cd docs && npm run gen:pages`；纯函数 `buildModulePage({ category, module, headers, sources })` 供脚本与测试共用（headers/sources 为模块目录下发现的文件名数组）

- [ ] **Step 1: 写失败测试**

```js
// docs/scripts/__tests__/gen-module-pages.spec.mjs
import { describe, it, expect } from 'vitest'
import { buildModulePage } from '../gen-module-pages.mjs'

const mod = {
  category: '01_线性表',
  module: '01_静态顺序表',
  headers: ['static_seq_list.h'],
  sources: ['static_seq_list.c', 'main.c']
}

describe('buildModulePage', () => {
  it('frontmatter 与标题', () => {
    const md = buildModulePage(mod)
    expect(md).toMatch(/^---\noutline: deep\n---\n\n# 01_静态顺序表\n/)
  })
  it('嵌码指令相对路径指向仓库源文件', () => {
    const md = buildModulePage(mod)
    expect(md).toContain('<<< ../01_线性表/01_静态顺序表/static_seq_list.h')
    expect(md).toContain('<<< ../01_线性表/01_静态顺序表/static_seq_list.c')
    expect(md).toContain('<<< ../01_线性表/01_静态顺序表/main.c')
  })
  it('含六大模板节与占位提示', () => {
    const md = buildModulePage(mod)
    for (const sec of ['## 一句话定位', '## 核心概念', '## 复杂度表', '## 关键代码', '## 易错点 / 考点', '## 动手跑'])
      expect(md).toContain(sec)
    expect(md).toContain('<!-- 待填 -->')
  })
  it('动手跑给出该分类的 cd 路径', () => {
    expect(buildModulePage(mod)).toContain('cd 01_线性表/01_静态顺序表')
  })
})
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run scripts/__tests__/gen-module-pages.spec.mjs`
Expected: FAIL（模块不存在）。

- [ ] **Step 3: 实现脚本**

```js
// docs/scripts/gen-module-pages.mjs
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(repoRoot, 'docs')

export function buildModulePage({ category, module: mod, headers, sources }) {
  const embed = files => files.map(f => `<<< ../${category}/${mod}/${f}`).join('\n\n')
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
```

- [ ] **Step 4: 跑测试确认通过，然后全量生成**

Run: `cd docs && npx vitest run scripts/__tests__/gen-module-pages.spec.mjs`
Expected: PASS ×4。

Run: `cd docs && npm run gen:pages && npm run docs:build`
Expected: 输出"生成 65 页，跳过 0 页"；build 成功（dead links 检查通过，因为首页 features 与侧边栏引用的 `/0X_分类/` 尚无 index.md 会怎样——注意：`link: /01_线性表/` 对应 `docs/01_线性表/index.md`，脚本已生成占位 index.md，所以不产生死链）。

- [ ] **Step 5: Commit**

```bash
git add docs/scripts docs/01_线性表 docs/02_栈 docs/03_队列 docs/04_特殊矩阵压缩存储 docs/05_串 docs/06_树 docs/07_图 docs/08_查找 docs/09_排序
git commit -m "feat: 骨架页生成脚本，全量生成 65 个模块页与分类占位页"
```

---

### Task 5: guide 两页 + 9 个分类概述页（手写内容）

**Files:**
- Create: `docs/guide/learning-path.md`
- Create: `docs/guide/conventions.md`
- Modify: `docs/01_线性表/index.md` … `docs/09_排序/index.md`（9 个占位页改为正式概述）

**Interfaces:**
- Consumes: README.md 的"建议学习顺序"与"贯穿全部模块的工程规范"两节（内容源）；Task 4 生成的分类 index.md 占位

- [ ] **Step 1: 学习路线页**（正文取自 README.md 第 29~43 行表格并改写为页面口吻）

```md
# 学习路线（零基础 · 约一个月）

没基础也完全能学：本库每个模块都独立自包含、可单独编译运行，**逐个啃即可**。
建议按难度从低到高推进，先吃透"范式"（不透明指针、错误处理、destroy 语义、
注释里的"为什么"），再逐个刷算法。分类里挑代表模块读透即可，
不必逐行读完全部 65 个模块。

| 阶段 | 内容 | 重心 |
|---|---|---|
| 第1周 | C 基础 + 01 线性表 + 02 栈 / 03 队列 | 读得懂 .h/.c 分离与状态码，会写插入/删除/查找 |
| 第2周 | 04 矩阵 + 05 串(KMP) + 06 树 | 递归与非递归遍历、线索化、哈夫曼 |
| 第3周 | 07 图 | DFS/BFS、最小生成树、最短路、拓扑/关键路径 |
| 第4周 | 08 查找 + 09 排序 | BST/AVL/红黑/B 树、内外排序与对拍 |

> 每天能挤多少算多少，关键是**每个模块亲自动手复现一遍核心函数**，效果远好于只看。
> 并行忙其他事也没关系：模块互不依赖，随时暂停随时接上；看不懂的先跳过，回头再补。
```

- [ ] **Step 2: 工程规范页**（正文取自 README.md 第 54~74 行十条规范，逐条展开）

```md
# 贯穿全部模块的工程规范（学习重点）

1. **头文件守卫**：三段式 `#ifndef / #define / #endif`，防重复包含
2. **接口/实现分离**：`.h` 只放声明，`.c` 放全部实现
3. **不透明指针**：头文件仅 `typedef struct Xxx Xxx;`，结构体定义藏在 `.c` 中，
   外部只能通过 API 操作——C 语言实现封装的核心手法
   （例外：定长数组且无隐藏不变量的结构——静态顺序表/静态链表/顺序栈/
   共享栈/循环顺序队列——直接在 .h 暴露布局，头文件注释均援引此条款）
4. **统一状态码**：各模块自带一份 `DsStatus` + `DsResult`（`code` + `message`），
   枚举数值从头一致（DS_OK = 0 起），成员按模块所需裁剪——查找类含
   DS_NOT_FOUND、树/图类只保留用得上的几值，不用裸返回值
5. **destroy 语义约定**：堆对象（init 用 `T**`）销毁用 `void xxx_destroy(T **obj)`，
   释放后自动置 NULL；栈对象（init 用 `T*`）销毁仅复位字段。
   调用方从此不必担心悬空指针
6. **const 正确性**：只读参数与打印函数一律 `const` 限定
7. **static 最小暴露**：实现内部辅助函数全部 `static`，不污染全局命名空间
8. **类前缀命名**：同类结构共用前缀（`list_` / `stack_` / `queue_` / `matrix_` /
   `seqlist_`），类型名大驼峰，函数名蛇形小写
9. **malloc 惯用法**：统一 `p = malloc(sizeof *p);` 无强转写法
10. **教学型注释**：文件头讲设计要点与复杂度，API 注释含
    @brief/@param/@return/@note 四要素，实现注释解释"为什么"与易错点
```

- [ ] **Step 3: 9 个分类概述页**

每页统一结构（三段式，300 字左右）：**是什么/解决什么问题 → 现实用途 → 学习建议与模块清单导览**。以下给出 01 的完整示例，其余 8 个分类按同一模板填写——内容数据见各页末尾要点表（取自 README.md 目录导航表"常用于"列与模块清单）：

```md
# 01 线性表

## 是什么

线性表是 n 个数据元素的有限序列，是最基本、最常用的线性结构。
本分类从"静态顺序表"到"静态链表"共 6 个模块，覆盖顺序存储与链式存储两大流派，
以及单/双/循环三种链路形态。

## 现实用途

一切"顺序/列表"数据：播放列表、浏览历史、通讯录。

## 学习建议

先读静态顺序表建立"下标 + 长度"的顺序存储直觉，再用动态顺序表理解扩容；
然后按 单链表 → 双链表 → 循环链表(含约瑟夫环) → 静态链表 的顺序啃链式实现，
重点体会不透明指针与 destroy 语义（见[工程规范](/guide/conventions)）。
```

其余 8 页的填写要点（每页"是什么"按此一句话定位展开、"现实用途"逐字使用）：

| 分类 | 一句话定位（"是什么"段落核心） | 现实用途（逐字） |
|---|---|---|
| 02_栈 | 后进先出（LIFO）的受限线性表；含两栈共享空间与括号匹配/表达式求值/递归三个应用模块 | 后进先出：函数调用栈、撤销/重做、浏览器后退、括号与表达式求值 |
| 03_队列 | 先进先出（FIFO）的受限线性表；循环顺序队列解决假溢出，含舞伴问题应用 | 先进先出：打印/任务排队、消息队列、银行叫号 |
| 04_特殊矩阵压缩存储 | 把有规律分布或大量零元素的矩阵压进一维数组省内存；三元组与十字链表存稀疏矩阵 | 省内存地存稀疏/规则数据：图像、图的邻接关系、稀疏矩阵运算 |
| 05_串 | 内容受限的线性表（字符）；重点是朴素匹配与 KMP(next+nextval) | 文本处理：搜索匹配(KMP)、查找替换、DNA 序列比对、日志关键字检索 |
| 06_树 | 一对多层级结构；从三种存储到二叉树遍历/线索化/哈夫曼/并查集 | 层级/关系数据：文件系统目录、DOM 树、编译器语法树、哈夫曼压缩、数据库 B 树索引、并查集(连通性) |
| 07_图 | 多对多网络结构；四种存储 + 遍历/MST/最短路/拓扑/关键路径全家桶 | 网络/关系建模：地图导航(最短路)、社交网络、物流(最小生成树/关键路径)、依赖调度(拓扑) |
| 08_查找 | 为"从海量数据快速定位"服务的结构族：BST/AVL/红黑/B/B+/散列 | 从海量数据快速定位：数据库/字典索引、登录校验、缓存、倒排索引、哈希表 |
| 09_排序 | 让数据有序以支撑查找与展示；内排序九种 + 外部排序（败者树/置换选择/最佳归并树） | 让数据有序以支撑查找与展示：排行榜、按价/时间排序、数据库 ORDER BY、海量数据外部排序 |

- [ ] **Step 4: 构建验证**

Run: `cd docs && npm run docs:build && npm run docs:preview`
Expected: build 成功；浏览器打开预览地址，首页 9 张卡、侧边栏两级树、9 个分类页、2 个 guide 页全部可达，`/guide/learning-path` 与 nav 对应。

- [ ] **Step 5: Commit**

```bash
git add docs/guide
git add 'docs/01_线性表/index.md' 'docs/02_栈/index.md' 'docs/03_队列/index.md' 'docs/04_特殊矩阵压缩存储/index.md' 'docs/05_串/index.md' 'docs/06_树/index.md' 'docs/07_图/index.md' 'docs/08_查找/index.md' 'docs/09_排序/index.md'
git commit -m "docs: 学习路线、工程规范两页与 9 个分类概述页"
```

---

### Task 6: 文档完整性检查脚本（TDD）

**Files:**
- Create: `docs/scripts/check-docs.mjs`
- Create: `docs/scripts/__tests__/check-docs.spec.mjs`

**Interfaces:**
- Consumes: Task 4 的目录约定（源模块目录 ↔ `docs/0X_*/0Y_模块.md` 一一对应）
- Produces: `cd docs && npm run check`；纯函数 `collectIssues(repoRoot, docsDir): string[]`（空数组 = 通过；脚本 main() 有问题时 `process.exit(1)`）

- [ ] **Step 1: 写失败测试**

```js
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
  fs.mkdirSync(docs, { recursive: true })
  fs.writeFileSync(path.join(docs, '01_线性表', '01_静态顺序表.md'),
    '<<< ../01_线性表/01_静态顺序表/a.h\n<<< ../01_线性表/01_静态顺序表/不存在.c\n')
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
})
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run scripts/__tests__/check-docs.spec.mjs`
Expected: FAIL。

- [ ] **Step 3: 实现脚本**

```js
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
        const target = path.resolve(path.dirname(md), m[1])
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
```

- [ ] **Step 4: 跑测试确认通过，再跑真实检查**

Run: `cd docs && npx vitest run scripts/__tests__/check-docs.spec.mjs && cd docs && npm run check`
Expected: 测试 PASS ×3；真实检查通过（Task 4/5 完成后无缺页，嵌码路径全部有效）。

- [ ] **Step 5: Commit**

```bash
git add docs/scripts/check-docs.mjs docs/scripts/__tests__/check-docs.spec.mjs
git commit -m "feat: 文档完整性检查脚本（缺页清单 + 嵌码路径校验）"
```

---

### Task 7: 动画类型定义 + usePlayer 播放器逻辑（TDD）

**Files:**
- Create: `docs/.vitepress/theme/visualizer/types.ts`
- Create: `docs/.vitepress/theme/visualizer/usePlayer.ts`
- Create: `docs/.vitepress/theme/visualizer/__tests__/usePlayer.spec.ts`

**Interfaces:**
- Produces（后续所有动画任务的契约）:

```ts
// types.ts
export interface Step {
  state: unknown                    // 渲染器解释的场景状态（arrayBar: number[] 或桶形态）
  highlights: number[]              // 已确定/高亮的下标
  active: [number, number] | null   // 当前比较/交换的一对下标
  narration: string                 // 该步解说
}
export interface VisualizerDef {
  title: string
  renderer: 'arrayBar'
  parse: (text: string) => unknown
  validate: (input: unknown) => string | null   // null=合法，否则错误文案
  steps: (input: any) => Step[]
  defaultInput: string
}
```

`usePlayer(stepsRef: Ref<Step[]>): { index: Ref<number>; playing: Ref<boolean>; speed: Ref<number>; current: ComputedRef<Step | null>; total: ComputedRef<number>; play; pause; toggle; next; prev; seek(i: number); setSpeed(ms: number) }`

- [ ] **Step 1: 写失败测试**

```ts
// docs/.vitepress/theme/visualizer/__tests__/usePlayer.spec.ts
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { ref, computed } from 'vue'
import { usePlayer } from '../usePlayer'
import type { Step } from '../types'

const mk = (n: number): Step[] =>
  Array.from({ length: n }, (_, i) => ({ state: [i], highlights: [], active: null, narration: `第${i}步` }))

beforeEach(() => vi.useFakeTimers())
afterEach(() => vi.useRealTimers())

describe('usePlayer', () => {
  it('初始停在第一帧，current 暴露当前 Step', () => {
    const p = usePlayer(ref(mk(3)))
    expect(p.index.value).toBe(0)
    expect(p.current.value!.narration).toBe('第0步')
    expect(p.total.value).toBe(3)
  })
  it('next/prev/seek 边界钳制', () => {
    const p = usePlayer(ref(mk(3)))
    p.next(); p.next()
    expect(p.index.value).toBe(2)
    p.next() // 末帧再 next 不越界
    expect(p.index.value).toBe(2)
    p.prev()
    expect(p.index.value).toBe(1)
    p.seek(99); expect(p.index.value).toBe(2)
    p.seek(-5); expect(p.index.value).toBe(0)
  })
  it('play 按 speed 自动前进，到末帧自动停', () => {
    const p = usePlayer(ref(mk(3)))
    p.setSpeed(100)
    p.play()
    expect(p.playing.value).toBe(true)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(1)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(2)
    vi.advanceTimersByTime(100)
    expect(p.index.value).toBe(2)
    expect(p.playing.value).toBe(false)
  })
  it('末帧再 play 从头开始；toggle 可暂停', () => {
    const p = usePlayer(ref(mk(2)))
    p.seek(1); p.play()
    expect(p.index.value).toBe(0)
    p.toggle()
    expect(p.playing.value).toBe(false)
  })
  it('steps 变化（换输入重算）时 index 重置到 0', () => {
    const steps = ref(mk(3))
    const p = usePlayer(steps)
    p.seek(2)
    steps.value = mk(5)
    expect(p.index.value).toBe(0)
    expect(p.total.value).toBe(5)
  })
  it('setSpeed 立即生效（播放中不重置进度）', () => {
    const p = usePlayer(ref(mk(4)))
    p.setSpeed(200); p.play()
    vi.advanceTimersByTime(200)
    expect(p.index.value).toBe(1)
    p.setSpeed(50) // 播放中调速
    expect(p.index.value).toBe(1)
    vi.advanceTimersByTime(50)
    expect(p.index.value).toBe(2)
  })
})
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/usePlayer.spec.ts`
Expected: FAIL（usePlayer 不存在）。

- [ ] **Step 3: 实现 types.ts + usePlayer.ts**

```ts
// docs/.vitepress/theme/visualizer/types.ts
export interface Step {
  state: unknown
  highlights: number[]
  active: [number, number] | null
  narration: string
}

export interface VisualizerDef {
  title: string
  renderer: 'arrayBar'
  parse: (text: string) => unknown
  validate: (input: unknown) => string | null
  steps: (input: any) => Step[]
  defaultInput: string
}
```

```ts
// docs/.vitepress/theme/visualizer/usePlayer.ts
import { ref, computed, watch, onUnmounted, getCurrentInstance } from 'vue'
import type { Ref, ComputedRef } from 'vue'
import type { Step } from './types'

export function usePlayer(steps: Ref<Step[]>) {
  const index = ref(0)
  const playing = ref(false)
  const speed = ref(400)
  let timer: ReturnType<typeof setInterval> | null = null

  const total: ComputedRef<number> = computed(() => steps.value.length)
  const current = computed(() => steps.value[index.value] ?? null)

  function stopTimer() { if (timer) { clearInterval(timer); timer = null } }
  function pause() { playing.value = false; stopTimer() }
  function play() {
    if (index.value >= steps.value.length - 1) index.value = 0
    playing.value = true
    stopTimer()
    timer = setInterval(() => {
      if (index.value < steps.value.length - 1) index.value++
      else pause()
    }, speed.value)
  }
  function toggle() { playing.value ? pause() : play() }
  function next() { if (index.value < steps.value.length - 1) index.value++; else pause() }
  function prev() { if (index.value > 0) index.value-- }
  function seek(i: number) { index.value = Math.min(Math.max(0, i), steps.value.length - 1) }
  function setSpeed(ms: number) {
    speed.value = ms
    if (playing.value) { stopTimer(); timer = setInterval(() => {
      if (index.value < steps.value.length - 1) index.value++; else pause()
    }, speed.value) }
  }

  watch(steps, () => { index.value = 0 })
  if (getCurrentInstance()) onUnmounted(stopTimer) // 测试环境无组件实例时不挂

  return { index, playing, speed, current, total, play, pause, toggle, next, prev, seek, setSpeed }
}
```

- [ ] **Step 4: 跑测试确认通过**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/usePlayer.spec.ts`
Expected: PASS ×6。

- [ ] **Step 5: Commit**

```bash
git add docs/.vitepress/theme/visualizer/types.ts docs/.vitepress/theme/visualizer/usePlayer.ts docs/.vitepress/theme/visualizer/__tests__/usePlayer.spec.ts
git commit -m "feat: 动画 Step 类型与 usePlayer 播放器逻辑（TDD）"
```

---

### Task 8: 排序步骤生成器 A——插入 / 希尔 / 冒泡 / 快排（TDD）

**Files:**
- Create: `docs/.vitepress/theme/visualizer/steps/insertionSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/shellSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/bubbleSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/quickSort.ts`
- Create: `docs/.vitepress/theme/visualizer/__tests__/steps.common.ts`
- Create: `docs/.vitepress/theme/visualizer/__tests__/sortA.spec.ts`

**Interfaces:**
- Consumes: `Step`（Task 7）
- Produces: 四个生成器，签名统一 `(input: number[]) => Step[]`；快排额外导出 `partitionSteps` 供 narration 复用不需要——仅默认导出 `xxxSortSteps`。共享断言 helper `expectSortedSteps(steps, input)` 与 `expectFrameInvariants(steps, len)`

- [ ] **Step 1: 写共享断言 helper 与失败测试**

```ts
// docs/.vitepress/theme/visualizer/__tests__/steps.common.ts
import { expect } from 'vitest'
import type { Step } from '../types'

export function expectSortedSteps(steps: Step[], input: number[]) {
  const last = steps[steps.length - 1]
  expect(last.state).toEqual([...input].sort((a, b) => a - b))
  expect(last.narration).toContain('完成')
}

export function expectFrameInvariants(steps: Step[], len: number) {
  expect(steps.length).toBeGreaterThan(1)
  for (const s of steps) {
    expect(s.narration.length).toBeGreaterThan(0)
    expect((s.state as number[]).length).toBe(len)          // 每帧数组长度不变
    for (const i of [...s.highlights, ...(s.active ?? [])]) {
      expect(i).toBeGreaterThanOrEqual(0)
      expect(i).toBeLessThan(len)                            // 高亮不越界
    }
  }
}
```

```ts
// docs/.vitepress/theme/visualizer/__tests__/sortA.spec.ts
import { describe, it, expect } from 'vitest'
import { insertionSortSteps } from '../steps/insertionSort'
import { shellSortSteps } from '../steps/shellSort'
import { bubbleSortSteps } from '../steps/bubbleSort'
import { quickSortSteps } from '../steps/quickSort'
import { expectSortedSteps, expectFrameInvariants } from './steps.common'

const cases: [string, (a: number[]) => any, number[][]][] = [
  ['insertion', insertionSortSteps, [[5, 2, 9, 1, 7], [1], [3, 3, 1], [4, 3, 2, 1]]],
  ['shell', shellSortSteps, [[8, 5, 3, 9, 1, 6], [2, 1]]],
  ['bubble', bubbleSortSteps, [[5, 2, 9, 1, 7], [1, 2, 3], [3, 2, 1]]],
  ['quick', quickSortSteps, [[5, 2, 9, 1, 7, 3], [1], [2, 1], [4, 4, 4, 1]]]
]

describe.each(cases)('%s 生成器', (_name, fn, inputs) => {
  it.each(inputs.map(i => [i]))('输入 %j 最终帧有序', (input) => {
    const steps = fn(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
  })
  it('纯函数：不修改输入', () => {
    const input = [3, 1, 2]
    fn([...input])
    const steps = fn(input)
    expect(input).toEqual([3, 1, 2])
    expect(steps[0].state).toEqual([3, 1, 2]) // 首帧=初始状态
  })
})

describe('bubble 附加行为', () => {
  it('已有序输入触发提前结束（narration 含"提前结束"）', () => {
    const steps = bubbleSortSteps([1, 2, 3])
    expect(steps.some(s => s.narration.includes('提前结束'))).toBe(true)
  })
})
describe('quick 附加行为', () => {
  it('narration 提及 pivot 与区间', () => {
    const steps = quickSortSteps([5, 2, 9, 1])
    expect(steps.some(s => s.narration.includes('pivot'))).toBe(true)
  })
})
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/sortA.spec.ts`
Expected: FAIL（四个 steps 模块不存在）。

- [ ] **Step 3: 实现四个生成器**

```ts
// docs/.vitepress/theme/visualizer/steps/insertionSort.ts
import type { Step } from '../types'

export function insertionSortSteps(input: number[]): Step[] {
  const a = [...input]
  const steps: Step[] = [{ state: [...a], highlights: [0], active: null, narration: `初始数组 [${a.join(', ')}]；认为 a[0] 已就位` }]
  for (let i = 1; i < a.length; i++) {
    const key = a[i]
    steps.push({ state: [...a], highlights: rangeExclusive(0, i), active: null, narration: `取出待插入元素 key = a[${i}] = ${key}` })
    let j = i - 1
    while (j >= 0 && a[j] > key) {
      steps.push({ state: [...a], highlights: [...rangeExclusive(0, i), i], active: [j, i], narration: `a[${j}] = ${a[j]} > key = ${key}，a[${j}] 后移一位` })
      a[j + 1] = a[j]
      steps.push({ state: [...a], highlights: [...rangeExclusive(0, i), i], active: [j, j + 1], narration: `后移完成 → [${a.join(', ')}]` })
      j--
    }
    if (j >= 0) steps.push({ state: [...a], highlights: [...rangeExclusive(0, i + 1)], active: [j, i], narration: `a[${j}] = ${a[j]} ≤ key，插入位置定为 ${j + 1}` })
    a[j + 1] = key
    steps.push({ state: [...a], highlights: rangeExclusive(0, i + 1), active: null, narration: `key 插入 a[${j + 1}] → 前 ${i + 1} 个元素有序` })
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function rangeExclusive(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/bubbleSort.ts
import type { Step } from '../types'

export function bubbleSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]，共 ${n} 个元素` }]
  const done = () => a.map((_, k) => k)
  for (let i = 0; i < n - 1; i++) {
    let swapped = false
    for (let j = 0; j < n - 1 - i; j++) {
      steps.push({ state: [...a], highlights: range(n - i, n), active: [j, j + 1], narration: `第 ${i + 1} 轮：比较 a[${j}]=${a[j]} 与 a[${j + 1}]=${a[j + 1]}` })
      if (a[j] > a[j + 1]) {
        ;[a[j], a[j + 1]] = [a[j + 1], a[j]]
        swapped = true
        steps.push({ state: [...a], highlights: range(n - i, n), active: [j, j + 1], narration: `逆序，交换 → [${a.join(', ')}]` })
      }
    }
    if (!swapped) {
      steps.push({ state: [...a], highlights: done(), active: null, narration: '本轮无交换，已整体有序，提前结束' })
      break
    }
    steps.push({ state: [...a], highlights: range(n - 1 - i, n), active: null, narration: `第 ${i + 1} 轮结束，最大值冒泡到位（尾部 ${i + 1} 个已有序）` })
  }
  steps.push({ state: [...a], highlights: done(), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/shellSort.ts
import type { Step } from '../types'

export function shellSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；希尔排序按增量分组做插入排序` }]
  for (let gap = Math.floor(n / 2); gap > 0; gap = Math.floor(gap / 2)) {
    steps.push({ state: [...a], highlights: [], active: null, narration: `增量 gap = ${gap}：相距 ${gap} 的元素为一组` })
    for (let i = gap; i < n; i++) {
      const key = a[i]
      let j = i - gap
      while (j >= 0 && a[j] > key) {
        steps.push({ state: [...a], highlights: [], active: [j, j + gap], narration: `gap=${gap}：比较 a[${j}]=${a[j]} 与 key=a[${i}]=${key}，a[${j}] 后移` })
        a[j + gap] = a[j]
        j -= gap
      }
      if (j + gap !== i) steps.push({ state: [...a], highlights: [], active: [j + gap, i], narration: `gap=${gap}：key=${key} 插入 a[${j + gap}]` })
      a[j + gap] = key
      steps.push({ state: [...a], highlights: [], active: null, narration: `gap=${gap} 本步完成 → [${a.join(', ')}]` })
    }
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/quickSort.ts
import type { Step } from '../types'

export function quickSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；快排用显式栈做 Lomuto 分区` }]
  const stack: [number, number][] = [[0, n - 1]]
  while (stack.length) {
    const [lo, hi] = stack.pop()!
    if (lo >= hi) {
      if (lo === hi) steps.push({ state: [...a], highlights: [lo], active: null, narration: `区间 [${lo}, ${hi}] 单元素，天然有序` })
      continue
    }
    const pivot = a[hi]
    steps.push({ state: [...a], highlights: [hi], active: null, narration: `区间 [${lo}, ${hi}]：pivot = a[${hi}] = ${pivot}` })
    let i = lo
    for (let j = lo; j < hi; j++) {
      steps.push({ state: [...a], highlights: [hi, ...range(lo, i)], active: [j, hi], narration: `j=${j}：a[${j}]=${a[j]} 与 pivot=${pivot} 比较` })
      if (a[j] < pivot) {
        if (i !== j) {
          ;[a[i], a[j]] = [a[j], a[i]]
          steps.push({ state: [...a], highlights: [hi, ...range(lo, i + 1)], active: [i, j], narration: `小于 pivot，换到左侧 → [${a.join(', ')}]` })
        }
        i++
      }
    }
    if (i !== hi) {
      ;[a[i], a[hi]] = [a[hi], a[i]]
      steps.push({ state: [...a], highlights: [...range(lo, i), i, ...range(i + 1, hi + 1)], active: [i, hi], narration: `pivot 归位到 a[${i}] → [${a.join(', ')}]` })
    }
    steps.push({ state: [...a], highlights: [...range(lo, i), i, ...range(i + 1, hi + 1)], active: null, narration: `pivot ${pivot} 就位：左 [${lo}, ${i - 1}]，右 [${i + 1}, ${hi}] 入栈` })
    stack.push([lo, i - 1], [i + 1, hi])
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

- [ ] **Step 4: 跑测试确认通过**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/sortA.spec.ts`
Expected: PASS（含 each 展开的全部用例）。

- [ ] **Step 5: Commit**

```bash
git add docs/.vitepress/theme/visualizer/steps docs/.vitepress/theme/visualizer/__tests__
git commit -m "feat: 排序步骤生成器 A——插入/希尔/冒泡/快排（TDD）"
```

---

### Task 9: 排序步骤生成器 B——选择 / 堆排 / 归并 / 计数 / 基数（TDD）

**Files:**
- Create: `docs/.vitepress/theme/visualizer/steps/selectionSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/heapSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/mergeSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/countingSort.ts`
- Create: `docs/.vitepress/theme/visualizer/steps/radixSort.ts`
- Create: `docs/.vitepress/theme/visualizer/__tests__/sortB.spec.ts`

**Interfaces:**
- Consumes: `Step`（Task 7）；`expectSortedSteps` / `expectFrameInvariants`（Task 8）
- Produces: 五个生成器，签名统一 `(input: number[]) => Step[]`。**计数/基数的 `state` 用桶形态** `{ array: number[]; buckets: number[][]; bucketLabels: string[] }`——共享 helper `expectSortedSteps` 对这两种生成器不适用（末帧 state 为桶形态），计数/基数改用本地断言"末帧 array 有序"

- [ ] **Step 1: 写失败测试**

```ts
// docs/.vitepress/theme/visualizer/__tests__/sortB.spec.ts
import { describe, it, expect } from 'vitest'
import { selectionSortSteps } from '../steps/selectionSort'
import { heapSortSteps } from '../steps/heapSort'
import { mergeSortSteps } from '../steps/mergeSort'
import { countingSortSteps } from '../steps/countingSort'
import { radixSortSteps } from '../steps/radixSort'
import { expectSortedSteps, expectFrameInvariants } from './steps.common'

describe('selection', () => {
  it.each([[[5, 2, 9, 1, 7]], [[3, 3, 1]], [[1]]])('%j 最终帧有序', (input) => {
    const steps = selectionSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
    expect(steps.some(s => s.narration.includes('最小'))).toBe(true)
  })
})

describe('heap', () => {
  it.each([[[5, 2, 9, 1, 7, 3]], [[1]], [[2, 1]]])('%j 最终帧有序', (input) => {
    const steps = heapSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
    expect(steps.some(s => s.narration.includes('建堆'))).toBe(true)
  })
})

describe('merge', () => {
  it.each([[[5, 2, 9, 1, 7, 3]], [[1]], [[2, 1]]])('%j 最终帧有序', (input) => {
    const steps = mergeSortSteps(input)
    expectSortedSteps(steps, input)
    expectFrameInvariants(steps, input.length)
  })
})

describe('counting（值域 0~9，桶形态）', () => {
  it('末帧 array 有序', () => {
    const steps = countingSortSteps([3, 1, 4, 1, 5])
    const last = steps[steps.length - 1].state as any
    expect(last.array).toEqual([1, 1, 3, 4, 5])
    expect(last.bucketLabels).toEqual(['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'])
  })
  it('纯函数：不修改输入', () => {
    const input = [3, 1, 2]
    countingSortSteps(input)
    expect(input).toEqual([3, 1, 2])
  })
})

describe('radix（值域 0~99，LSD 十位分桶）', () => {
  it('末帧 array 有序', () => {
    const steps = radixSortSteps([42, 7, 91, 7, 30])
    const last = steps[steps.length - 1].state as any
    expect(last.array).toEqual([7, 7, 30, 42, 91])
  })
  it('出现个位轮与十位轮两轮分发', () => {
    const steps = radixSortSteps([42, 7, 91])
    expect(steps.some(s => s.narration.includes('个位'))).toBe(true)
    expect(steps.some(s => s.narration.includes('十位'))).toBe(true)
  })
})
```

- [ ] **Step 2: 跑测试确认失败**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/sortB.spec.ts`
Expected: FAIL。

- [ ] **Step 3: 实现五个生成器**

```ts
// docs/.vitepress/theme/visualizer/steps/selectionSort.ts
import type { Step } from '../types'

export function selectionSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]` }]
  for (let i = 0; i < n - 1; i++) {
    let min = i
    for (let j = i + 1; j < n; j++) {
      steps.push({ state: [...a], highlights: range(0, i), active: [min, j], narration: `无序区最小暂为 a[${min}]=${a[min]}，与 a[${j}]=${a[j]} 比较` })
      if (a[j] < a[min]) {
        min = j
        steps.push({ state: [...a], highlights: range(0, i), active: [min], narration: `更小，最小下标更新为 ${min}` })
      }
    }
    if (min !== i) {
      ;[a[i], a[min]] = [a[min], a[i]]
      steps.push({ state: [...a], highlights: range(0, i + 1), active: [i, min], narration: `最小值 ${a[i]} 换到 a[${i}]` })
    } else {
      steps.push({ state: [...a], highlights: range(0, i + 1), active: null, narration: `a[${i}] 本就是无序区最小，不动` })
    }
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/heapSort.ts
import type { Step } from '../types'

export function heapSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；父 i 的孩子是 2i+1、2i+2` }]

  function siftDown(start: number, size: number) {
    let parent = start
    while (true) {
      const l = 2 * parent + 1, r = 2 * parent + 2
      let max = parent
      if (l < size && a[l] > a[max]) max = l
      if (r < size && a[r] > a[max]) max = r
      if (max === parent) {
        steps.push({ state: [...a], highlights: range(size, n), active: [parent], narration: `a[${parent}]=${a[parent]} 不小于孩子，下沉停止` })
        return
      }
      steps.push({ state: [...a], highlights: range(size, n), active: [parent, max], narration: `孩子 a[${max}]=${a[max]} 更大，与 a[${parent}]=${a[parent]} 交换` })
      ;[a[parent], a[max]] = [a[max], a[parent]]
      parent = max
    }
  }

  steps.push({ state: [...a], highlights: [], active: null, narration: '阶段一：自最后一个非叶节点起建大顶堆' })
  for (let i = Math.floor(n / 2) - 1; i >= 0; i--) siftDown(i, n)
  steps.push({ state: [...a], highlights: [], active: null, narration: `建堆完成 → [${a.join(', ')}]` })
  for (let size = n - 1; size > 0; size--) {
    steps.push({ state: [...a], highlights: range(size + 1, n), active: [0, size], narration: `堆顶 ${a[0]} 与末尾 a[${size}]=${a[size]} 交换，堆大小缩为 ${size}` })
    ;[a[0], a[size]] = [a[size], a[0]]
    steps.push({ state: [...a], highlights: range(size, n), active: [0], narration: `a[${size}] 就位，新堆顶下沉调整` })
    siftDown(0, size)
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/mergeSort.ts
import type { Step } from '../types'

export function mergeSortSteps(input: number[]): Step[] {
  const a = [...input]
  const n = a.length
  const steps: Step[] = [{ state: [...a], highlights: [], active: null, narration: `初始数组 [${a.join(', ')}]；自底向上归并：宽 1,2,4…` }]
  const buf = new Array<number>(n)
  for (let width = 1; width < n; width *= 2) {
    for (let lo = 0; lo < n - width; lo += 2 * width) {
      const mid = lo + width, hi = Math.min(lo + 2 * width, n)
      steps.push({ state: [...a], highlights: range(lo, hi), active: null, narration: `合并有序段 [${lo}, ${mid - 1}] 与 [${mid}, ${hi - 1}]` })
      let i = lo, j = mid, k = lo
      while (i < mid && j < hi) {
        steps.push({ state: [...a], highlights: range(lo, hi), active: [i, j], narration: `比较 a[${i}]=${a[i]} 与 a[${j}]=${a[j]}，取${a[i] <= a[j] ? `a[${i}]` : `a[${j}]`}` })
        buf[k++] = a[i] <= a[j] ? a[i++] : a[j++]
      }
      while (i < mid) buf[k++] = a[i++]
      while (j < hi) buf[k++] = a[j++]
      for (let t = lo; t < hi; t++) a[t] = buf[t]
      steps.push({ state: [...a], highlights: range(lo, hi), active: null, narration: `段 [${lo}, ${hi - 1}] 归并完成 → [${a.join(', ')}]` })
    }
  }
  steps.push({ state: [...a], highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/countingSort.ts
import type { Step } from '../types'

// 值域 0~9；state 为桶形态 { array, buckets, bucketLabels }
export function countingSortSteps(input: number[]): Step[] {
  const a = [...input]
  const K = 10
  const buckets: number[][] = Array.from({ length: K }, () => [])
  const bucketLabels = Array.from({ length: K }, (_, k) => String(k))
  const frame = (arr: number[], narration: string, active: [number, number] | null = null, highlights: number[] = []): Step =>
    ({ state: { array: [...arr], buckets: buckets.map(b => [...b]), bucketLabels }, highlights, active, narration })
  const steps: Step[] = [frame(a, `初始数组 [${a.join(', ')}]；值域 0~9，用计数桶`)]
  for (let i = 0; i < a.length; i++) {
    buckets[a[i]].push(a[i])
    steps.push(frame(a, `C[${a[i]}]++：值 ${a[i]} 入桶 ${a[i]}`, [i, i]))
  }
  steps.push(frame(a, '计数完成，按桶序收集'))
  const out: number[] = []
  for (let v = 0; v < K; v++) {
    for (const _ of buckets[v]) {
      out.push(v)
      steps.push(frame([...out, ...a.slice(out.length)], `桶 ${v} 收集 → out[${out.length - 1}] = ${v}`, [out.length - 1, out.length - 1], range(0, out.length)))
    }
  }
  steps.push({ state: { array: out, buckets: buckets.map(b => [...b]), bucketLabels }, highlights: out.map((_, k) => k), active: null, narration: `排序完成：[${out.join(', ')}]` })
  return steps
}

function range(lo: number, hi: number): number[] {
  const r: number[] = []
  for (let k = lo; k < hi; k++) r.push(k)
  return r
}
```

```ts
// docs/.vitepress/theme/visualizer/steps/radixSort.ts
import type { Step } from '../types'

// 值域 0~99，LSD 基数排序（个位 → 十位）
export function radixSortSteps(input: number[]): Step[] {
  const a = [...input]
  const buckets: number[][] = Array.from({ length: 10 }, () => [])
  const bucketLabels = Array.from({ length: 10 }, (_, k) => String(k))
  const frame = (arr: number[], narration: string, active: [number, number] | null = null, highlights: number[] = []): Step =>
    ({ state: { array: [...arr], buckets: buckets.map(b => [...b]), bucketLabels }, highlights, active, narration })
  const steps: Step[] = [frame(a, `初始数组 [${a.join(', ')}]；LSD 基数排序：先个位后十位`)]

  let pass = 0
  for (const exp of [1, 10]) {
    const digitName = exp === 1 ? '个位' : '十位'
    for (let i = 0; i < a.length; i++) {
      const d = Math.floor(a[i] / exp) % 10
      buckets[d].push(a[i])
      pass++
      steps.push(frame(a, `第 ${pass} 轮（${digitName}）：${a[i]} 的 ${digitName}是 ${d}，入桶 ${d}`, [i, i]))
    }
    steps.push(frame(a, `${digitName}分发完成，按桶 0→9 依次收集`))
    const merged: number[] = []
    for (const b of buckets) { merged.push(...b); b.length = 0 }
    for (let i = 0; i < merged.length; i++) a[i] = merged[i]
    steps.push(frame(a, `${digitName}收集完成 → [${a.join(', ')}]`))
  }
  steps.push({ state: { array: [...a], buckets: buckets.map(b => [...b]), bucketLabels }, highlights: a.map((_, k) => k), active: null, narration: `排序完成：[${a.join(', ')}]` })
  return steps
}
```

- [ ] **Step 4: 跑测试确认通过**

Run: `cd docs && npx vitest run .vitepress/theme/visualizer/__tests__/sortB.spec.ts`
Expected: PASS。

- [ ] **Step 5: Commit**

```bash
git add docs/.vitepress/theme/visualizer/steps docs/.vitepress/theme/visualizer/__tests__/sortB.spec.ts
git commit -m "feat: 排序步骤生成器 B——选择/堆排/归并/计数/基数（TDD）"
```

---

### Task 10: PlayerShell + ArrayBar 渲染器 + Visualizer 组件 + 算法注册表

**Files:**
- Create: `docs/.vitepress/theme/visualizer/renderers/ArrayBar.vue`
- Create: `docs/.vitepress/theme/visualizer/PlayerShell.vue`
- Create: `docs/.vitepress/theme/visualizer/registry.ts`
- Create: `docs/.vitepress/theme/visualizer/Visualizer.vue`
- Create: `docs/.vitepress/theme/index.ts`
- Create: `docs/guide/visualizer-demo.md`

**Interfaces:**
- Consumes: `usePlayer`（Task 7）、9 个生成器（Task 8/9）
- Produces: 全局组件 `<Visualizer algorithm="quick-sort" />`（kebab-case 注册名 `Visualizer`，prop `algorithm` 取 registry 键）；registry 键：`insertion-sort` / `shell-sort` / `bubble-sort` / `quick-sort` / `selection-sort` / `heap-sort` / `merge-sort` / `counting-sort` / `radix-sort`
- 测试策略：交互逻辑已在 usePlayer 被 Vitest 覆盖；SFC 渲染不引 `@vue/test-utils`（spec 依赖约束），以 demo 页 + `docs:build` + 人工/截图验证代替

- [ ] **Step 1: ArrayBar 渲染器**

```vue
<!-- docs/.vitepress/theme/visualizer/renderers/ArrayBar.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import type { Step } from '../types'

const props = defineProps<{ step: Step | null }>()

interface View { array: number[]; buckets: number[][]; bucketLabels: string[] }

const view = computed<View | null>(() => {
  if (!props.step) return null
  const s = props.step.state
  if (Array.isArray(s)) return { array: s as number[], buckets: [], bucketLabels: [] }
  const o = s as any
  if (o && Array.isArray(o.array)) return { array: o.array, buckets: o.buckets ?? [], bucketLabels: o.bucketLabels ?? [] }
  return null
})

const maxVal = computed(() => Math.max(1, ...(view.value?.array ?? [1])))

function cls(i: number) {
  const s = props.step
  if (!s) return ''
  if (s.active && (s.active[0] === i || s.active[1] === i)) return 'bar active'
  if (s.highlights.includes(i)) return 'bar done'
  return 'bar'
}

const H_MIN = 16, H_MAX = 150
const h = (v: number) => H_MIN + Math.round((v / maxVal.value) * (H_MAX - H_MIN))
</script>

<template>
  <div v-if="view" class="arraybar">
    <div class="row">
      <div v-for="(v, i) in view.array" :key="i" :class="cls(i)">
        <div class="fill" :style="{ height: h(v) + 'px' }"></div>
        <span class="val">{{ v }}</span>
        <span class="idx">{{ i }}</span>
      </div>
    </div>
    <div v-if="view.buckets.length" class="buckets">
      <div v-for="(b, bi) in view.buckets" :key="bi" class="bucket">
        <div class="bucket-label">{{ view.bucketLabels[bi] }}</div>
        <div class="bucket-items"><span v-for="(x, xi) in b" :key="xi">{{ x }}</span></div>
      </div>
    </div>
    <div class="legend">
      <span class="chip"><i class="swatch done"></i>已就位</span>
      <span class="chip"><i class="swatch active"></i>当前比较/交换</span>
      <span class="chip"><i class="swatch"></i>未处理</span>
    </div>
  </div>
</template>

<style scoped>
.arraybar { padding: 12px 0; }
.row { display: flex; align-items: flex-end; gap: 6px; min-height: 190px; flex-wrap: wrap; }
.bar { display: flex; flex-direction: column; justify-content: flex-end; align-items: center; width: 34px; border-radius: 4px 4px 0 0; }
.fill { width: 100%; background: var(--vp-c-indigo-2); border-radius: 3px 3px 0 0; transition: height .25s, background .25s; }
.bar.done .fill { background: var(--vp-c-green-2); }
.bar.active .fill { background: var(--vp-c-yellow-2); }
.val { font-size: 12px; margin-top: 2px; font-variant-numeric: tabular-nums; }
.idx { font-size: 10px; opacity: .55; }
.buckets { display: flex; gap: 6px; margin-top: 14px; flex-wrap: wrap; }
.bucket { border: 1px solid var(--vp-c-divider); border-radius: 6px; min-width: 40px; padding: 4px 6px; text-align: center; }
.bucket-label { font-size: 11px; opacity: .6; }
.bucket-items { display: flex; gap: 4px; justify-content: center; min-height: 20px; font-size: 12px; }
.legend { display: flex; gap: 14px; margin-top: 12px; font-size: 12px; opacity: .8; }
.swatch { display: inline-block; width: 12px; height: 12px; border-radius: 3px; background: var(--vp-c-bg-soft); border: 1px solid var(--vp-c-divider); vertical-align: -2px; }
.swatch.done { background: var(--vp-c-green-2); border: none; }
.swatch.active { background: var(--vp-c-yellow-2); border: none; }
</style>
```

- [ ] **Step 2: PlayerShell 播放器**

```vue
<!-- docs/.vitepress/theme/visualizer/PlayerShell.vue -->
<script setup lang="ts">
import { ref, computed } from 'vue'
import { usePlayer } from './usePlayer'
import type { VisualizerDef } from './types'

const props = defineProps<{ def: VisualizerDef }>()

const inputText = ref(props.def.defaultInput)
const error = ref<string | null>(null)

const parsed = computed(() => {
  error.value = null
  try {
    const input = props.def.parse(inputText.value)
    const err = props.def.validate(input)
    if (err) { error.value = err; return null }
    return input
  } catch {
    error.value = '输入格式不正确'
    return null
  }
})

const steps = computed(() => (parsed.value === null ? [] : props.def.steps(parsed.value)))
const player = usePlayer(steps)
const { index, playing, speed, current, total } = player

const speedOptions = [ { label: '0.5×', ms: 800 }, { label: '1×', ms: 400 }, { label: '2×', ms: 200 }, { label: '4×', ms: 100 } ]
</script>

<template>
  <div class="player">
    <div class="title">{{ def.title }}</div>
    <div class="stage"><slot :current="current" /></div>
    <div v-if="current" class="narration">{{ current.narration }}</div>
    <div v-if="error" class="err">⚠ {{ error }}（示例输入：{{ def.defaultInput }}）</div>
    <div class="controls">
      <button @click="player.prev()" :disabled="index === 0">⏮ 上一步</button>
      <button class="primary" @click="player.toggle()" :disabled="!total"> {{ playing ? '⏸ 暂停' : '▶ 播放' }} </button>
      <button @click="player.next()" :disabled="index >= total - 1">下一步 ⏭</button>
      <input class="range" type="range" min="0" :max="Math.max(0, total - 1)" :value="index"
             @input="player.seek(Number(($event.target as HTMLInputElement).value))" :disabled="!total" />
      <span class="counter">{{ total ? index + 1 : 0 }} / {{ total }}</span>
      <select :value="String(speed)" @change="player.setSpeed(Number(($event.target as HTMLSelectElement).value))">
        <option v-for="o in speedOptions" :key="o.ms" :value="String(o.ms)">{{ o.label }}</option>
      </select>
    </div>
    <div class="input-row">
      <label>输入数据：</label>
      <input v-model="inputText" placeholder="如 5,2,9,1,7" />
    </div>
  </div>
</template>

<style scoped>
.player { border: 1px solid var(--vp-c-divider); border-radius: 10px; padding: 14px 16px; margin: 12px 0; background: var(--vp-c-bg-soft); }
.title { font-weight: 600; margin-bottom: 8px; }
.narration { margin-top: 10px; padding: 8px 10px; border-radius: 6px; background: var(--vp-c-bg); font-size: 14px; min-height: 20px; }
.err { color: var(--vp-c-danger-1); font-size: 13px; margin-top: 6px; }
.controls { display: flex; align-items: center; gap: 8px; margin-top: 10px; flex-wrap: wrap; }
.controls button { padding: 4px 10px; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg); cursor: pointer; }
.controls button.primary { background: var(--vp-c-brand-1); color: #fff; border: none; }
.controls button:disabled { opacity: .45; cursor: not-allowed; }
.range { flex: 1; min-width: 120px; }
.counter { font-size: 12px; opacity: .7; font-variant-numeric: tabular-nums; }
.input-row { display: flex; align-items: center; gap: 8px; margin-top: 10px; font-size: 13px; }
.input-row input { flex: 1; padding: 4px 8px; border: 1px solid var(--vp-c-divider); border-radius: 6px; background: var(--vp-c-bg); color: var(--vp-c-text-1); }
</style>
```

- [ ] **Step 3: registry 注册表 + Visualizer 组件 + 主题入口**

```ts
// docs/.vitepress/theme/visualizer/registry.ts
import type { VisualizerDef } from './types'
import { insertionSortSteps } from './steps/insertionSort'
import { shellSortSteps } from './steps/shellSort'
import { bubbleSortSteps } from './steps/bubbleSort'
import { quickSortSteps } from './steps/quickSort'
import { selectionSortSteps } from './steps/selectionSort'
import { heapSortSteps } from './steps/heapSort'
import { mergeSortSteps } from './steps/mergeSort'
import { countingSortSteps } from './steps/countingSort'
import { radixSortSteps } from './steps/radixSort'

function numberList(min: number, max: number, maxLen: number) {
  return {
    parse: (text: string) => text.split(/[,，\s]+/).filter(s => s.length).map(Number),
    validate: (input: unknown): string | null => {
      if (!Array.isArray(input) || input.length === 0) return '至少输入 1 个数字'
      if (input.length > maxLen) return `最多 ${maxLen} 个元素`
      if (input.some(v => !Number.isInteger(v) || v < min || v > max)) return `值需为 ${min}~${max} 的整数`
      return null
    }
  }
}

const sortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 999, 20)
})
const bucketSortDef = (title: string, steps: (a: number[]) => any, defaultInput: string): VisualizerDef => ({
  title, renderer: 'arrayBar', steps, defaultInput, ...numberList(0, 99, 20)
})

export const registry: Record<string, VisualizerDef> = {
  'insertion-sort': sortDef('插入排序', insertionSortSteps, '5,2,9,1,7'),
  'shell-sort': sortDef('希尔排序', shellSortSteps, '8,5,3,9,1,6'),
  'bubble-sort': sortDef('冒泡排序', bubbleSortSteps, '5,2,9,1,7'),
  'quick-sort': sortDef('快速排序', quickSortSteps, '5,2,9,1,7,3'),
  'selection-sort': sortDef('简单选择排序', selectionSortSteps, '5,2,9,1,7'),
  'heap-sort': sortDef('堆排序', heapSortSteps, '5,2,9,1,7,3'),
  'merge-sort': sortDef('归并排序', mergeSortSteps, '5,2,9,1,7,3'),
  'counting-sort': bucketSortDef('计数排序（值域 0~9）', countingSortSteps, '3,1,4,1,5'),
  'radix-sort': bucketSortDef('基数排序（LSD，值域 0~99）', radixSortSteps, '42,7,91,7,30')
}
```

```vue
<!-- docs/.vitepress/theme/visualizer/Visualizer.vue -->
<script setup lang="ts">
import { computed } from 'vue'
import { registry } from './registry'
import PlayerShell from './PlayerShell.vue'
import ArrayBar from './renderers/ArrayBar.vue'

const props = defineProps<{ algorithm: string }>()
const def = computed(() => registry[props.algorithm])
</script>

<template>
  <div v-if="def">
    <PlayerShell :def="def" v-slot="{ current }">
      <ArrayBar :step="current" />
    </PlayerShell>
  </div>
  <p v-else style="color: var(--vp-c-danger-1)">未知动画：{{ props.algorithm }}（registry 中不存在）</p>
</template>
```

```ts
// docs/.vitepress/theme/index.ts
import DefaultTheme from 'vitepress/theme'
import Visualizer from './visualizer/Visualizer.vue'

export default {
  extends: DefaultTheme,
  enhanceApp({ app }) {
    app.component('Visualizer', Visualizer)
  }
}
```

- [ ] **Step 4: demo 页验证（构建 + 预览）**

创建 `docs/guide/visualizer-demo.md`：

```md
# 动画演示 Demo

冒泡排序（本页为开发验证页，随站点保留作为动画组件示例）：

<Visualizer algorithm="bubble-sort" />

基数排序（桶形态渲染）：

<Visualizer algorithm="radix-sort" />
```

Run: `cd docs && npx vitest run && npm run docs:build && npm run docs:preview`
Expected: 全部单测 PASS；build 成功；预览页中两个动画可播放、单步、调速、改输入重算、非法输入（如 `5,abc,2000`）出现校验提示。

- [ ] **Step 5: Commit**

```bash
git add docs/.vitepress/theme docs/guide/visualizer-demo.md
git commit -m "feat: 动画基建——PlayerShell/ArrayBar/Visualizer 与 9 种排序注册"
```

---

### Task 11: 9 个排序模块页挂动画

**Files:**
- Modify: `docs/09_排序/01_插入排序.md`、`02_希尔排序.md`、`03_冒泡排序.md`、`04_快速排序.md`、`05_简单选择排序.md`、`06_堆排序.md`、`07_归并排序.md`、`08_计数排序.md`、`09_基数排序.md`（以 `cd docs && npm run gen:pages` 生成的实际文件名为准——先 `ls docs/09_排序/` 确认，模块目录名是权威来源）

**Interfaces:**
- Consumes: registry 键（Task 10）、骨架页"## 动画演示"节（Task 4 模板）

- [ ] **Step 1: 确认实际文件名**

Run: `ls docs/09_排序/`
Expected: 9 个模块页 + index.md（名称以源码目录为准）。

- [ ] **Step 2: 逐页替换动画占位**

每页把骨架模板中的：

```md
## 动画演示

<!-- 该模块暂无动画；动画基建完成后按批次挂 <Visualizer algorithm="..." /> -->
```

替换为（映射：01→insertion-sort、02→shell-sort、03→bubble-sort、04→quick-sort、05→selection-sort、06→heap-sort、07→merge-sort、08→counting-sort、09→radix-sort，按 Step 1 的实际文件名对号）：

```md
## 动画演示

下面动画支持播放/单步/调速，也可以改成你自己的数据（1~20 个、0~999 的整数）：

<Visualizer algorithm="quick-sort" />
```

（第 8/9 页提示语改为"值域 0~99"。）

- [ ] **Step 3: 构建与预览验证**

Run: `cd docs && npm run check && npm run test && npm run docs:build && npm run docs:preview`
Expected: check 通过、单测全 PASS、build 成功；预览中 9 个排序页动画均正常。

- [ ] **Step 4: Commit**

```bash
git add docs/09_排序
git commit -m "feat: 9 个排序模块页挂载动画（V1 批次完成）"
```

---

### Task 12: README 站点说明 + 全量验证收尾

**Files:**
- Modify: `README.md`（顶部新增一节，不动其余内容）

- [ ] **Step 1: README 增加站点入口**

在 `# DataStructure — C 语言数据结构工程实现` 标题段之后插入：

```md
## 🖥️ 教程站点（本地预览）

本项目配有 VitePress 教程站：65 个模块的讲解、源码展示与排序算法动画。

```bash
cd docs && npm install
npm run docs:dev      # http://localhost:5173
```

其他命令：`cd docs && npm run docs:build`（构建静态产物）、`cd docs && npm run test`（动画步骤单测）、
`cd docs && npm run check`（讲解页完整性检查）、`cd docs && npm run gen:pages`（为新模块生成骨架页）。
```

- [ ] **Step 2: 全量验证**

Run: `cd docs && npm run check && npm run test && npm run docs:build`
Expected: 三条全部通过；`docs/.vitepress/dist/` 生成完整站点。

- [ ] **Step 3: 验收对照（spec 验收标准 1~7）**

逐条核对 spec"验收标准"节：1 站点启动 ✓ 2 骨架页+嵌码 ✓ 3 侧边栏对应 ✓ 4 动画基建+V1 ✓ 5 搜索/暗色/移动端（VitePress 内置，预览确认）✓ 6 check 脚本 ✓ 7 C 代码零改动（`git status --short 01_* 02_* 03_* 04_* 05_* 06_* 07_* 08_* 09_*` 应无输出）。

- [ ] **Step 4: Commit**

```bash
git add README.md
git commit -m "docs: README 增加教程站点使用说明"
```

---

## 后续批次（不在本计划内，实施完成后另行立计划）

按 spec"首批动画清单"：V2 线性类（listNode/stackQueue 渲染器）→ V3 树类（tree 渲染器）→ V4 图（graph 渲染器）→ V5 串与查找（string 渲染器）。每批 = 新渲染器 + 新 steps 生成器 + 对应模块页挂载，复用本计划的基建。
