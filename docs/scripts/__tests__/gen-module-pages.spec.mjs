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
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/static_seq_list.h')
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/static_seq_list.c')
    expect(md).toContain('<<< @/../01_线性表/01_静态顺序表/main.c')
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

const appMod = {
  category: '02_栈',
  module: '04_栈的应用',
  headers: ['01_括号匹配/bracket_match.h', '02_表达式求值/expression_eval.h', '03_递归/recursion.h'],
  sources: ['01_括号匹配/bracket_match.c', '01_括号匹配/main.c', '03_递归/recursion.c', '03_递归/main.c']
}

describe('buildModulePage 应用集合模块（源码嵌套在子目录）', () => {
  it('接口节按子目录分组嵌 .h，全部 .c 照常嵌码', () => {
    const md = buildModulePage(appMod)
    expect(md).toContain('### 01_括号匹配\n\n<<< @/../02_栈/04_栈的应用/01_括号匹配/bracket_match.h')
    expect(md).toContain('### 02_表达式求值\n\n<<< @/../02_栈/04_栈的应用/02_表达式求值/expression_eval.h')
    expect(md).not.toContain('无统一接口头文件')
    expect(md).toContain('<<< @/../02_栈/04_栈的应用/01_括号匹配/bracket_match.c')
    expect(md).toContain('<<< @/../02_栈/04_栈的应用/01_括号匹配/main.c')
  })
  it('动手跑按子目录逐个编译，不再用顶层 *.c', () => {
    const md = buildModulePage(appMod)
    expect(md).toContain('cd 02_栈/04_栈的应用')
    expect(md).toContain('gcc -Wall -Wextra -std=c99 01_括号匹配/*.c -o demo.exe && ./demo.exe')
    expect(md).toContain('gcc -Wall -Wextra -std=c99 03_递归/*.c -o demo.exe && ./demo.exe')
    expect(md).not.toContain('gcc -Wall -Wextra -std=c99 *.c')
  })
  it('复杂度表占位不引用未实现的 ComplexityTable 组件（扁平模块同样生效）', () => {
    for (const m of [mod, appMod]) {
      const md = buildModulePage(m)
      expect(md).not.toContain('ComplexityTable')
      expect(md).toContain('建议用 Markdown 表格呈现（操作 × 时间/空间复杂度）')
    }
  })
})
