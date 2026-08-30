import { describe, it, expect } from 'vitest'
import { registry } from '../registry'

describe('registry 输入校验', () => {
  it('全部算法（9 排序 + 16 批1 + 10 批2 + 12 批3 + 15 批4）注册齐全且 defaultInput 均通过自身校验', () => {
    const keys = Object.keys(registry)
    expect(keys).toHaveLength(62)
    for (const k of keys) {
      const def = registry[k]
      const parsed = def.parse(def.defaultInput)
      expect(def.validate(parsed)).toBeNull()
    }
  })
  it('parse 非数字片段产生 NaN，validate 拒绝', () => {
    const def = registry['bubble-sort']
    expect(def.validate(def.parse('5,abc,2'))).not.toBeNull()
  })
  it('counting-sort 值域 0~9：拒 10 与 99，收 9', () => {
    const def = registry['counting-sort']
    expect(def.validate([10])).not.toBeNull()
    expect(def.validate([99])).not.toBeNull()
    expect(def.validate([9])).toBeNull()
  })
  it('radix-sort 值域 0~99：收 99 拒 100', () => {
    const def = registry['radix-sort']
    expect(def.validate([99])).toBeNull()
    expect(def.validate([100])).not.toBeNull()
  })
  it('长度上限 20：21 个元素拒绝，空输入拒绝', () => {
    const def = registry['bubble-sort']
    expect(def.validate(Array.from({ length: 21 }, () => 1))).not.toBeNull()
    expect(def.validate([])).not.toBeNull()
  })
})
