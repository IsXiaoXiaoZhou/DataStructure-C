// 60 模块页逐页校验：身份 / 覆盖 / 嵌码指向 / 语义锚点 / 动画键
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..')
const docsDir = path.join(root, 'docs')

// README 命名速查：每模块期望的函数前缀/类型名（语义锚点）
const EXPECT = {
  '01_线性表/01_静态顺序表': ['seqlist_', 'SeqList'],
  '01_线性表/02_动态顺序表': ['seqlist_', 'SeqList'],
  '01_线性表/03_单链表': ['list_', 'SinglyLinkedList'],
  '01_线性表/04_双链表': ['list_', 'DoublyLinkedList'],
  '01_线性表/05_循环链表': ['list_', 'CircularLinkedList'],
  '01_线性表/06_静态链表': ['list_', 'StaticLinkedList'],
  '02_栈/01_顺序栈': ['stack_', 'SeqStack'],
  '02_栈/02_两栈共享空间': ['stack_', 'SharedStack'],
  '02_栈/03_链栈': ['stack_', 'LinkStack'],
  '02_栈/04_栈的应用': ['bracket_match', 'infix_to_postfix'],
  '03_队列/01_循环顺序队列': ['queue_', 'SeqQueue'],
  '03_队列/02_链队列': ['queue_', 'LinkQueue'],
  '03_队列/03_队列的应用': ['dance_partner'],
  '04_特殊矩阵压缩存储/01_对称矩阵': ['matrix_', 'SymmetricMatrix'],
  '04_特殊矩阵压缩存储/02_三角矩阵': ['matrix_', 'TriangularMatrix'],
  '04_特殊矩阵压缩存储/03_对角矩阵': ['matrix_', 'DiagonalMatrix'],
  '04_特殊矩阵压缩存储/04_稀疏矩阵-三元组': ['matrix_', 'Triple'],
  '04_特殊矩阵压缩存储/05_稀疏矩阵-十字链表': ['matrix_', 'Cross'],
  '05_串/01_定长顺序串': ['ss_', 'StaticString'],
  '05_串/02_堆分配串': ['hs_', 'HeapString'],
  '05_串/03_块链串': ['bs_', 'BlockString'],
  '05_串/04_朴素模式匹配': ['bf_'],
  '05_串/05_KMP算法': ['kmp_'],
  '06_树/01_树的存储结构': ['pt_', 'ParentTree'],
  '06_树/02_二叉树顺序存储': ['sbt_', 'SeqBinaryTree'],
  '06_树/03_二叉树链式实现': ['bitree_', 'BiTree'],
  '06_树/04_线索二叉树': ['tt_', 'ThreadTree'],
  '06_树/05_森林与二叉树转换': ['forest_', 'Forest'],
  '06_树/06_哈夫曼树': ['ht_', 'HuffmanTree'],
  '06_树/07_并查集': ['uf_', 'UnionFind'],
  '07_图/01_邻接矩阵': ['mg_', 'MatrixGraph'],
  '07_图/02_邻接表': ['ag_', 'AdjListGraph'],
  '07_图/03_十字链表': ['og_', 'OrthGraph'],
  '07_图/04_邻接多重表': ['aml_', 'AmlGraph'],
  '07_图/05_最小生成树': ['mst_', 'MstGraph'],
  '07_图/06_最短路径': ['sp_', 'SpGraph'],
  '07_图/07_拓扑排序': ['topo_', 'TopoGraph'],
  '07_图/08_关键路径': ['cp_', 'AoeGraph'],
  '08_查找/01_顺序查找': ['search_', 'seq_search'],
  '08_查找/02_折半查找（插值查找、斐波那契查找）': ['search_', 'bin_search'],
  '08_查找/03_分块查找': ['blk_search_'],
  '08_查找/04_二叉排序树': ['bst_', 'BSTree'],
  '08_查找/05_平衡二叉树': ['avl_', 'AVLTree'],
  '08_查找/06_红黑树': ['rb_', 'RBTree'],
  '08_查找/07_B树': ['btree_', 'BTree'],
  '08_查找/08_B+树': ['bplus_'],
  '08_查找/09_散列函数': ['hash_'],
  '08_查找/10_散列表-拉链法': ['hash_', 'Chain'],
  '08_查找/11_散列表-开放定址法': ['hash_', 'OpenAddr'],
  '08_查找/12_散列查找性能分析': ['hash_'],
  '09_排序/01_插入排序': ['insert_sort'],
  '09_排序/02_希尔排序': ['shell_sort'],
  '09_排序/03_冒泡排序': ['bubble_sort'],
  '09_排序/04_快速排序': ['quick_sort'],
  '09_排序/05_简单选择排序': ['selection_sort'],
  '09_排序/06_堆排序': ['heap_sort'],
  '09_排序/07_归并排序': ['merge_sort'],
  '09_排序/08_计数排序': ['counting_sort'],
  '09_排序/09_基数排序': ['radix_sort'],
  '09_排序/10_外部排序': ['loser_tree', '置换选择'],
}

function listModuleFiles(dir, base = '') {
  const out = []
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const rel = base ? base + '/' + e.name : e.name
    if (e.isDirectory()) out.push(...listModuleFiles(path.join(dir, e.name), rel))
    else if (/\.(c|h)$/.test(e.name)) out.push(rel)
  }
  return out.sort()
}

const rows = []
const categories = fs.readdirSync(root).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(root, d)).isDirectory()).sort()
let total = 0
for (const cat of categories) {
  const mods = fs.readdirSync(path.join(root, cat)).filter(d => /^\d{2}_/.test(d) && fs.statSync(path.join(root, cat, d)).isDirectory()).sort()
  for (const mod of mods) {
    total++
    const key = cat + '/' + mod
    const mdPath = path.join(docsDir, cat, mod + '.md')
    const row = { key, checks: {}, fails: [] }
    rows.push(row)
    // C1: 页面存在
    if (!fs.existsSync(mdPath)) { row.checks.page = 'FAIL'; row.fails.push('页面不存在'); continue }
    row.checks.page = 'ok'
    const md = fs.readFileSync(mdPath, 'utf8')
    // C2: H1 身份
    const h1 = md.match(/^# (.+)$/m)?.[1]
    row.checks.h1 = h1 === mod ? 'ok' : 'FAIL(' + h1 + ')'
    if (h1 !== mod) row.fails.push('H1=' + h1)
    // C3: 嵌码目标 == 源文件全集（双向）
    const embeds = [...md.matchAll(/^<<<\s+@\/\.\.(.+)$/gm)].map(m => m[1].trim())
    const actual = listModuleFiles(path.join(root, cat, mod))
    const embedSet = new Set(embeds.map(e => e.replace(/^\//, '')))
    const actualSet = new Set(actual.map(f => cat + "/" + mod + "/" + f))
    const missing = [...actualSet].filter(f => !embedSet.has(f))
    const extra = [...embedSet].filter(f => !actualSet.has(f))
    row.checks.coverage = missing.length === 0 && extra.length === 0 ? 'ok(' + actual.length + '文件)' : 'FAIL(缺嵌' + missing.length + ' 错嵌' + extra.length + ')'
    if (missing.length) row.fails.push('漏嵌: ' + missing.join(', '))
    if (extra.length) row.fails.push('错嵌: ' + extra.join(', '))
    // C4: 语义锚点——嵌的 .h 内容含期望前缀/类型
    const expect = EXPECT[key] || []
    if (expect.length) {
      const hFiles = embeds.filter(f => f.endsWith('.h'))
      let allContent = ''
      for (const f of hFiles) {
        const ap = path.join(root, f)
        if (fs.existsSync(ap)) allContent += fs.readFileSync(ap, 'utf8')
      }
      const lc = allContent.toLowerCase(); const hit = expect.filter(e => lc.includes(e.toLowerCase()))
      row.checks.semantics = hit.length > 0 ? 'ok(' + hit.join('/') + ')' : 'FAIL(未命中 ' + expect.join('|') + ')'
      if (hit.length === 0) row.fails.push('语义锚点未命中: ' + expect.join('|'))
    } else {
      row.checks.semantics = 'skip(未配置锚点)'
    }
    // C5: 模板七节
    const sections = ['## 一句话定位', '## 核心概念', '## 复杂度表', '## 关键代码', '## 动画演示', '## 易错点 / 考点', '## 动手跑']
    const missSec = sections.filter(s => !md.includes(s))
    row.checks.template = missSec.length === 0 ? 'ok(7节)' : 'FAIL(缺' + missSec.join(',') + ')'
    if (missSec.length) row.fails.push('缺节: ' + missSec.join(','))
    // C6: 排序页动画键
    if (cat === '09_排序' && !md.includes('暂无动画')) {
      const ALGO = { '01': 'insertion-sort', '02': 'shell-sort', '03': 'bubble-sort', '04': 'quick-sort', '05': 'selection-sort', '06': 'heap-sort', '07': 'merge-sort', '08': 'counting-sort', '09': 'radix-sort' }
      const want = ALGO[mod.slice(0, 2)]
      if (want) {
        const got = md.match(/algorithm="([\w-]+)"/)?.[1]
        row.checks.visualizer = got === want ? 'ok(' + got + ')' : 'FAIL(有' + got + ' 应' + want + ')'
        if (got !== want) row.fails.push('动画键 ' + got + ' 应为 ' + want)
      }
    }
  }
}
// 输出逐页明细
let failCount = 0
for (const r of rows) {
  const ok = r.fails.length === 0
  if (!ok) failCount++
  console.log((ok ? 'PASS' : 'FAIL') + '  ' + r.key)
  if (!ok) r.fails.forEach(f => console.log('      - ' + f))
  else console.log('      覆盖' + r.checks.coverage + ' 语义' + (r.checks.semantics ?? '') + ' ' + (r.checks.visualizer ?? ''))
}
console.log('\n=== 共 ' + total + ' 页，PASS ' + (total - failCount) + '，FAIL ' + failCount + ' ===')
