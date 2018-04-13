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
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  auto& allocator = data->root_allocator;
  vulkan::LibraryWrapper wrapper(allocator, data->log.get());
  vulkan::VkInstance instance(
      vulkan::CreateEmptyOneDotOneInstance(allocator, &wrapper));
  vulkan::VkDevice device(vulkan::CreateDefaultDevice(allocator, instance));

  {  // 1. One bindings one element.
    VkDescriptorSetLayoutBinding bindings[3] = {
        {
            /* binding = */ 0,
            /* descriptorType = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            /* descriptorCount = */ 6,
            /* stageFlags = */
            (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
            /* pImmutableSamplers = */ nullptr,
        },
    };
    VkDescriptorSetLayoutCreateInfo layout_create_info{
        /* sType = */ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        /* pNext = */ nullptr,
        /* flags = */ 0,
        /* bindingCount = */ 1,
        /* pBindings = */ bindings,
    };

    ::VkDescriptorSetLayout layout;
    LOG_ASSERT(==, data->log,
               device->vkCreateDescriptorSetLayout(device, &layout_create_info,
                                                   nullptr, &layout),
               VK_SUCCESS);

    // Update descriptors at array index 3, 4 and 5, with arbitary stride value.
    VkDescriptorUpdateTemplateEntry entry{
        0,                                 // dstBinding
        3,                                 // dstArrayElement
        3,                                 // descriptorCount
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  // descriptorType
        0,                                 // offset
        24,                                // stride
    };
    VkDescriptorUpdateTemplateCreateInfo template_create_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkDescriptorUpdateTemplateCreateFlags(0),                  // flags
        1,       // descriptorUpdateEntryCount
        &entry,  // pDescriptorUpdateEntries
        VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,  // templateType
        layout,                            // descriptorSetLayout
        VK_PIPELINE_BIND_POINT_GRAPHICS,   // pipelineBindPoint
        VkPipelineLayout(VK_NULL_HANDLE),  // pipelineLayout
        0,                                 // set
    };

    ::VkDescriptorUpdateTemplate update_template;
    device->vkCreateDescriptorUpdateTemplate(device, &template_create_info,
                                             nullptr, &update_template);
    device->vkDestroyDescriptorUpdateTemplate(device, update_template, nullptr);

    device->vkDestroyDescriptorSetLayout(device, layout, nullptr);
  }

  {  // 2. Multiple entries.
    VkDescriptorSetLayoutBinding bindings[3] = {
        {
            /* binding = */ 0,
            /* descriptorType = */ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            /* descriptorCount = */ 6,
            /* stageFlags = */
            (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
            /* pImmutableSamplers = */ nullptr,
        },
        {
            /* binding = */ 2,
            /* descriptorType = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            /* descriptorCount = */ 3,
            /* stageFlags = */ VK_SHADER_STAGE_VERTEX_BIT,
            /* pImmutableSamplers = */ nullptr,
        },
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info{
        /* sType = */ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        /* pNext = */ nullptr,
        /* flags = */ 0,
        /* bindingCount = */ 3,
        /* pBindings = */ bindings,
    };

    ::VkDescriptorSetLayout layout;
    device->vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr,
                                        &layout);

    // Update descriptors at array index 3, 4 and 5
    VkDescriptorUpdateTemplateEntry entries[4]{
        // update storage image binding at index 0, 1 and 2, with arbitary
        // stride value
        {
            0,                                 // dstBinding
            0,                                 // dstArrayElement
            3,                                 // descriptorCount
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  // descriptorType
            0,                                 // offset
            24,                                // stride
        },
        // update storage image binding at index 5
        {
            0,                                 // dstBinding
            5,                                 // dstArrayElement
            1,                                 // descriptorCount
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  // descriptorType
            3 * 24,                            // offset
            24,                                // stride
        },
        // update uniform buffer binding at index 1 and 2
        {
            2,                                  // dstBinding
            1,                                  // dstArrayElement
            2,                                  // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
            256,                                // offset
            24,                                 // stride
        },
        // update uniform buffer binding at index 0
        {
            2,                                  // dstBinding
            0,                                  // dstArrayElement
            1,                                  // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
            512,                                // offset
            24,                                 // stride
        },
    };
    VkDescriptorUpdateTemplateCreateInfo template_create_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkDescriptorUpdateTemplateCreateFlags(0),                  // flags
        4,        // descriptorUpdateEntryCount
        entries,  // pDescriptorUpdateEntries
        VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,  // templateType
        layout,                            // descriptorSetLayout
        VK_PIPELINE_BIND_POINT_GRAPHICS,   // pipelineBindPoint
        VkPipelineLayout(VK_NULL_HANDLE),  // pipelineLayout
        0,                                 // set
    };

    ::VkDescriptorUpdateTemplate update_template;
    device->vkCreateDescriptorUpdateTemplate(device, &template_create_info,
                                             nullptr, &update_template);
    device->vkDestroyDescriptorUpdateTemplate(device, update_template, nullptr);

    device->vkDestroyDescriptorSetLayout(device, layout, nullptr);
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
