# vkCreateDescriptorUpdateTemplate / vkDestroyDescriptorUpdateTemplate

## Signatures
```c++
VkResult vkCreateDescriptorUpdateTemplate(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplate*                 pDescriptorUpdateTemplate);

void vkDestroyDescriptorUpdateTemplate(
    VkDevice                                    device,
    VkDescriptorUpdateTemplate                  descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator);
```

# VkDescriptorUpdateTemplateCreateInfo
```c++
typedef struct VkDescriptorUpdateTemplateCreateInfo {
    VkStructureType                           sType;
    void*                                     pNext;
    VkDescriptorUpdateTemplateCreateFlags     flags;
    uint32_t                                  descriptorUpdateEntryCount;
    const VkDescriptorUpdateTemplateEntry*    pDescriptorUpdateEntries;
    VkDescriptorUpdateTemplateType            templateType;
    VkDescriptorSetLayout                     descriptorSetLayout;
    VkPipelineBindPoint                       pipelineBindPoint;
    VkPipelineLayout                          pipelineLayout;
    uint32_t                                  set;
} VkDescriptorUpdateTemplateCreateInfo;

typedef struct VkDescriptorUpdateTemplateEntry {
    uint32_t            dstBinding;
    uint32_t            dstArrayElement;
    uint32_t            descriptorCount;
    VkDescriptorType    descriptorType;
    size_t              offset;
    size_t              stride;
} VkDescriptorUpdateTemplateEntry;
```

According to the Vulkan spec:
- `flags` **must** be 0
- `descriptorUpdateEntryCount` is the number of elements in the
  `pDescriptorUpdateEntries` array
- `pDescriptorUpdateEntries` is a pointer to an array of
  `VkDescriptorUpdateTemplateEntry` structures describing the descriptors to be
  updsated by the descriptor update template
- `templateType` specifies the type of the descriptor update template. If set
  to `VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET` it can **only** be
  used to update descriptor sets with a fixed `descriptorSetLayout`
- `descriptorSetLayout` is the descriptor set layout the parameter update
  template will be used with. All descriptor sets which are going to be updated
  through the newly created descriptor update template **must** be created with
  this layout. `descriptorSetLayout` is the descriptor set layout used to build
  the descriptor update template. All descriptor sets which are going to be
  updated through the newly created descriptor update template **must** be
  created with a layout that matches (is the same as, or defined identically
  to) this layout. This parameter is ignored if `templateType` is not
  `VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET`
- Both of `descriptorSetLayout`, and `pipelineLayout` that are valid handles
  **must** have been created, allocated, or retrieved from the same `VkDevice`
- If `descriptorSetLayout` is not `VK_NULL_HANDLE`, `descriptorSetLayout`
  **must** be a valid `VkDescriptorSetLayout` handle
- `descriptorUpdateEntryCount` **must** be greater than 0
- `dstBinding` **must** be a valid binding in the descriptor set layout
  implicitly specified when using a descriptor update template to update
  descriptors.
- `dstArrayElement` and `descriptorCount` **must** be less than or equal to the
  number of array elements in the descriptor set binding implicitly specified
  when using a descriptor update template to update descriptors, and all
  applicable consecutive bindings, as described by consecutive binding updates

For `vkCreateDescriptorUpdateTemplate`, these tests should test the following cases:
- [ ] `descriptorUpdateEntryCount` of value 1
- [ ] `descriptorUpdateEntryCount` of value greater than 1

For `vkDestroyDescriptorUpdateTemplate`, these tests should test the following cases:
- [ ] `descriptorUpdateTemplate` == `VK_NULL_HANDLE`
- [ ] `descriptorUpdateTemplate` != `VK_NULL_HANDLE`
