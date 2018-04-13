# vkUpdateDescriptorSetWithTemplate

## Signatures
```c++
void vkUpdateDescriptorSetWithTemplate(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const void*                                 pData);
```

# Valid usage
- pData **must** be a valid pointer to a memory that contains one or more valid
  instances of `VkDescriptorImageInfo`, `VkDescriptorBufferInfo` or
  `VkBufferView` in a layout defined by `descriptorUpdateTemplate` when it was
  created with `vkCreateDescriptorUpdateTemplate`

# Tests
- [ ] With `VkDescriptorImageInfo`, `VkDescriptorBufferInfo` and `VkBufferView`
