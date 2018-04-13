# Copyright 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from gapit_test_framework import gapit_test, GapitTest
from gapit_test_framework import require, require_equal, require_not_equal
from gapit_test_framework import little_endian_bytes_to_int
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, HANDLE, POINTER, DEVICE_SIZE

UPDATE_DATE_ELEMENTS = [
    # One VkDescriptorImageInfo
    ("sampler", HANDLE),
    ("imageView", HANDLE),
    ("imageLayout", UINT32_T),
    # Two VkDescriptorBufferInfo
    ("buffer_0", HANDLE),
    ("offset_0", DEVICE_SIZE),
    ("range_0", DEVICE_SIZE),
    ("buffer_1", HANDLE),
    ("offset_1", DEVICE_SIZE),
    ("range_1", DEVICE_SIZE),
    # Three VkBufferView
    ("buffer_view_0", HANDLE),
    ("buffer_view_1", HANDLE),
    ("buffer_view_2", HANDLE),
]


@gapit_test("vkUpdateDescriptorSetWithTemplate_test")
class OneImageInfoTwoBufferInfoThreeBufferViews(GapitTest):

    def expect(self):
        arch = self.architecture
        update_with_template = require(self.nth_call_of("vkUpdateDescriptorSetsWithTemplate", 1))
        require_not_equal(0, update_with_template.device)
        require_not_equal(0, update_with_template.descriptorSet)
        require_not_equal(0, update_with_template.descriptorUpdateTemplate)
        require_not_equal(0, update_with_template.pData)

        data = VulkanStruct(
            arch, UPDATE_DATE_ELEMENTS,
            lambda offset, size: little_endian_bytes_to_int(require(
                update_with_template.get_read_data(
                    update_with_template.hex_pData + offset, size))))
