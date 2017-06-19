/* Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "support/entry/entry.h"
#include "support/log/log.h"
#include "vulkan_helpers/helper_functions.h"
#include "vulkan_helpers/known_device_infos.h"
#include "vulkan_helpers/structs.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  auto& allocator = data->root_allocator;
  vulkan::LibraryWrapper wrapper(allocator, data->log.get());
  vulkan::VkInstance instance(
      vulkan::CreateEmptyInstance(data->root_allocator, &wrapper));
  vulkan::VkDevice device(vulkan::CreateDefaultDevice(allocator, instance));

  uint32_t memory_index =
      vulkan::GetMemoryIndex(&device, data->log.get(), 0xFFFFFFFFU,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                 VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
  data->log->LogInfo("Using memory index: ", memory_index);

  uint32_t heap_index = device.physical_device_memory_properties()
                            .memoryTypes[memory_index]
                            .heapIndex;
  VkDeviceSize heap_size =
      device.physical_device_memory_properties().memoryHeaps[heap_index].size;
  data->log->LogInfo("Using heap index: ", heap_index, " size: ", heap_size);

  VkDeviceSize memory_size = heap_size / 2u;
  data->log->LogInfo("Allocating memory size: ", memory_size);

  VkMemoryAllocateInfo allocate_info{
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
      nullptr,                                 // pNext
      memory_size,                             // allocationSize
      memory_index};

  VkDeviceMemory device_memory;
  LOG_ASSERT(==, data->log, VK_SUCCESS,
             device->vkAllocateMemory(device, &allocate_info, nullptr,
                                      &device_memory));

  VkDeviceSize committed_size = 0;
  device->vkGetDeviceMemoryCommitment(device, device_memory, &committed_size);
  data->log->LogInfo("Committed memory in bytes: ", committed_size);

  device->vkFreeMemory(device, device_memory, nullptr);

  data->log->LogInfo("Application Shutdown");
  return 0;
}
