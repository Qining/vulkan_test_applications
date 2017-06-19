# vkGetDeviceMemoryCommitment

## Signatures
```c++
void vkGetDeviceMemoryCommitment(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize*                               pCommittedMemoryInBytes);
```

According to the Vulkan spec:
- The implementation **may** update the commitment at any time, and the value
  returned by this query **may** be out of date.
- `memory` **must** have been create with a memory type that reports
  `VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`.
- `pCommittedMemoryInBytes` **must** be a pointer to `VkDeviceSize` value


These tests should test the following cases:
- [x] Valid `memory`
- [x] Valid `pCommittedMemoryInBytes`
