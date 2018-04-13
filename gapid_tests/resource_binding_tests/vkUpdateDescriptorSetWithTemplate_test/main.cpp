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
#include "vulkan_helpers/vulkan_application.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  auto& allocator = data->root_allocator;
  vulkan::VulkanApplication app(data->root_allocator, data->log.get(), data);
  vulkan::VkDevice& device = app.device();

  {
    // Update with one descriptor image info structs, two buffer info structs
    // and three buffer views.
    VkDescriptorSetLayoutBinding bindings[3]{
        {
            0,                                 // binding
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  // descriptorType
            1,                                 // descriptorCount
            VK_SHADER_STAGE_COMPUTE_BIT,       // stageFlags
            nullptr,                           // pImmutableSamplers
        },
        {
            1,                                  // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
            2,                                  // descriptorCount
            VK_SHADER_STAGE_VERTEX_BIT,         // stageFlags
            nullptr,                            // pImmutableSamplers
        },
        {
            2,                                        // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,  // descriptorType
            3,                                        // descriptorCount
            VK_SHADER_STAGE_FRAGMENT_BIT,             // stageFlags
            nullptr,                                  // pImmutableSamplers
        },
    };
    auto set =
        app.AllocateDescriptorSet({bindings[0], bindings[1], bindings[2]});

    // Underlying resources
    auto uniform_buffer = app.CreateAndBindDefaultExclusiveDeviceBuffer(
        512, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    auto texel_buffer = app.CreateAndBindDefaultExclusiveDeviceBuffer(
        1024, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    auto texel_buffer_view = app.CreateBufferView(
        *texel_buffer, VK_FORMAT_R32G32B32A32_UINT, 0, 256);
    VkImageCreateInfo img_create_info = VkImageCreateInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        0,                                    // flags
        VK_IMAGE_TYPE_2D,                     // imageType
        VK_FORMAT_R32G32B32A32_UINT,          // format
        {32, 32, 1},                          // extent
        1,                                    // mipLevels
        1,                                    // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                // samples
        VK_IMAGE_TILING_OPTIMAL,              // tiling
        VK_IMAGE_USAGE_STORAGE_BIT,           // usage
        VK_SHARING_MODE_EXCLUSIVE,            // sharing mode
        0,                                    // queueFamilyIndexCount
        nullptr,                              // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_GENERAL,              // initialLayout
    };
    auto img = app.CreateAndBindImage(&img_create_info);
    auto img_view =
        app.CreateImageView(img.get(), VK_IMAGE_VIEW_TYPE_2D,
                            VkImageSubresourceRange{
                                VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
                                0,                          // baseMipLevel
                                1,                          // levelCount
                                0,                          // baseArrayLayer
                                1,                          // layerCount
                            });

    const VkDescriptorImageInfo img_info = {
        VkSampler(VK_NULL_HANDLE),  // sampler
        *img_view,                  // imageView
        VK_IMAGE_LAYOUT_GENERAL,    // imageLayout
    };
    const VkDescriptorBufferInfo buf_info[2] = {
        {
            /* buffer = */ *uniform_buffer,
            /* offset = */ 0,
            /* range = */ 256,
        },
        {
            /* buffer = */ *uniform_buffer,
            /* offset = */ 256,
            /* range = */ 256,
        },
    };
    containers::vector<containers::unique_ptr<vulkan::VkBufferView>> buf_views(
        allocator);
    containers::vector<::VkBufferView> raw_buf_views(allocator);
    for (size_t i = 0; i < 3; i++) {
      buf_views.push_back(app.CreateBufferView(
          *texel_buffer, VK_FORMAT_R32G32B32A32_UINT, i * 256, 256));
      raw_buf_views.push_back(*buf_views[i].get());
    }

    // Prepare pData
    containers::vector<uint8_t> descriptor_data(allocator);
    descriptor_data.reserve(sizeof(VkDescriptorImageInfo) +
                            2 * sizeof(VkDescriptorBufferInfo) +
                            3 * sizeof(::VkBufferView));
    uint8_t* pData = descriptor_data.data();
    memcpy(pData, &img_info, sizeof(VkDescriptorImageInfo));
    pData += sizeof(VkDescriptorImageInfo);
    memcpy(pData, &buf_info, 2 * sizeof(VkDescriptorBufferInfo));
    pData += 2 * sizeof(VkDescriptorBufferInfo);
    memcpy(pData, raw_buf_views.data(), 3 * sizeof(::VkBufferView));
    pData = descriptor_data.data();

    // Create the template
    VkDescriptorUpdateTemplateEntry entries[3]{
        {0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0,
         sizeof(VkDescriptorImageInfo)},
        {1, 0, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         sizeof(VkDescriptorImageInfo), sizeof(VkDescriptorBufferInfo)},
        {2, 0, 3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         sizeof(VkDescriptorImageInfo) + 2 * sizeof(VkDescriptorBufferInfo),
         sizeof(::VkBufferView)},
    };
    VkDescriptorUpdateTemplateCreateInfo template_create_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        VkDescriptorUpdateTemplateCreateFlags(0),                  // flags
        3,        // descriptorUpdateEntryCount
        entries,  // pDescriptorUpdateEntries
        VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,  // templateType
        set.layout(),                      // descriptorSetLayout
        VK_PIPELINE_BIND_POINT_GRAPHICS,   // pipelineBindPoint
        VkPipelineLayout(VK_NULL_HANDLE),  // pipelineLayout
        0,                                 // set
    };
    ::VkDescriptorUpdateTemplate update_template;
    device->vkCreateDescriptorUpdateTemplate(device, &template_create_info,
                                             nullptr, &update_template);

    // Execute vkUpdateDescriptorSetWithTemplate
    device->vkUpdateDescriptorSetWithTemplate(device, set, update_template,
                                              pData);

    device->vkDestroyDescriptorUpdateTemplate(device, update_template, nullptr);
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
