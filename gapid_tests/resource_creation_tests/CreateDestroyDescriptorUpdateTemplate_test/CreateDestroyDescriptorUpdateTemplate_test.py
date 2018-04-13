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

from gapit_test_framework import gapit_test, require, require_equal
from gapit_test_framework import require_not_equal, little_endian_bytes_to_int
from gapit_test_framework import GapitTest
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, FLOAT, POINTER, HANDLE, SIZE_T

DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_ELEMENTS = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("descriptorUpdateEntryCount", UINT32_T),
    ("pDescriptorUpdateEntries", POINTER),
    ("templateType", UINT32_T),
    ("descriptorSetLayout", HANDLE),
    ("pipelineBindPoint", UINT32_T),
    ("pipelineLayout", HANDLE),
    ("set", UINT32_T),
]

DESCRIPTOR_UPDATE_TEMPLATE_ENTRY_ELEMENTS = [
    ("dstBinding", UINT32_T),
    ("dstArrayElement", UINT32_T),
    ("descriptorCount", UINT32_T),
    ("descriptorType", UINT32_T),
    ("offset", SIZE_T),
    ("sride", SIZE_T),
]


def check_create_descriptor_update_template(test, index):
    """Gets the |index|'th vkCreateDescriptorUpdateTemplate call atom, and
    checks its return value and arguments. Also checks the returned descriptor
    update template handle is not null. Returns the checked
    vkCreateDescriptorUpdateTemplate atom, device and descriptor update template
    handle. This method does not check the content of the
    VkDescriptorUpdateTemplateCreateInfo struct used in the
    vkCreateDescriptorUpdateTemplate call.  """
    create_template = require(test.nth_call_of(
        "vkCreateDescriptorUpdateTemplate", 1))
    require_equal(VK_SUCCESS, int(create_template.return_val))
    device = create_template.int_device
    require_not_equal(0, device)
    p_create_info = create_template.hex_pCreateInfo
    require_not_equal(0, p_create_info)
    p_template = create_template.hex_pDescriptorUpdateTemplate
    require_not_equal(0, p_template)
    template = little_endian_bytes_to_int(require(
        create_template.get_write_data(
            p_template, NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, template)
    return create_template, device, descriptor_update_template


def check_destroy_descriptor_update_template(test, device, descriptor_update_template):
    """Checks that the next vkDestroyDescriptorUpdateTemplate command call atom
    has the passed-in |device| and |descriptor_update_template| handle value.
    """
    destroy_descriptor_update_template = require(test.next_call_of(
        "vkDestroyDescriptorUpdateTemplate"))
    require_equal(device, destroy_descriptor_update_template.int_device)
    require_equal(descriptor_update_template,
                  destroy_descriptor_update_template.int_descriptorUpdateTemplate)


def get_descriptor_update_template_create_info(create_descriptor_update_template,
                                               architecture):
    """Returns a VulkanStruct representing the
    VkDescriptorUpdateTemplateCreateInfo struct used in the given
    |create_descriptor_update_template| command."""
    return VulkanStruct(
        architecture, DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO_ELEMENTS,
        lambda offset, size: little_endian_bytes_to_int(require(
            create_descriptor_update_template.get_read_data(
                create_descriptor_update_template.hex_pCreateInfo + offset, size))))


def get_entry(create_descriptor_update_template, architecture, create_info, index):
    """Returns a VulkanStruct representing the |index|'th (starting from 0)
    VkDescriptorUpdateTemplateEntry struct baked in |create_info| for the
    |create_descriptor_update_template| atom."""
    entry_offset = create_info.pDescriptorUpdateEntries + index * (
        # Ok, this is a little bit tricky. We have 4 32-bit integers and 2
        # size_t in VkDescriptorUpdateTemplate struct. On 32-bit
        # architecture, size_t size is 4, so alignement should be fine.
        # On 64-bit architecture, size_t size is 8, the struct totally
        # occupies (4 * 4 + 2 * 8) bytes, so alignment should also be fine.
        4 * 4 + 2 * int(architecture.int_sizeSize))
    return VulkanStruct(
        architecture, DESCRIPTOR_UPDATE_TEMPLATE_ENTRY_ELEMENTS,
        lambda offset, size: little_endian_bytes_to_int(
            require(create_descriptor_update_template.get_read_data(
                entry_offset + offset, size))))


@gapit_test("CreateDestroyDescriptorUpdateTemplate_test")
class OneEntry(GapitTest):
    def expect(self):
        """1. Single entry."""
        arch = self.architecture
        create_template, device, template = \
            check_create_descriptor_update_template(self, 1)
        info = get_descriptor_update_template_create_info(create_template, arch)
        require_equal(info.sType,
                      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO)
        require_equal(info.pNext, 0)
        require_equal(info.flags, 0)
        require_equal(info.descriptorUpdateEntryCount, 1)
        require_not_equal(info.pDescriptorUpdateEntries, 0)
        require_equal(info.templateType, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET)
        require_not_equal(info.descriptorSetLayout, 0)
        require_equal(info.pipelineBindPoint, 0)
        require_equal(info.pipelineLayout, 0)
        require_equal(info.set, 0)

        entry = get_entry(create_template, arch, info, 0)
        require_equal(entry.dstBinding, 0)
        require_equal(entry.dstArrayElement, 3)
        require_equal(entry.descriptorCount, 3)
        require_equal(entry.descriptorType, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        require_equal(entry.offset, 0)
        require_equal(entry.stride, 24)

        check_destroy_descriptor_update_template(self, device, template)


@gapit_test("CreateDestroyDescriptorUpdateTemplate_test")
class MultipleEntries(GapitTest):
    def expect(self):
        """2. Four entries."""
        arch = self.architecture
        create_template, device, template = \
            check_create_descriptor_update_template(self, 2)
        info = get_descriptor_update_template_create_info(create_template, arch)
        require_equal(info.sType,
                      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO)
        require_equal(info.pNext, 0)
        require_equal(info.flags, 0)
        require_equal(info.descriptorUpdateEntryCount, 4)
        require_not_equal(info.pDescriptorUpdateEntries, 0)
        require_equal(info.templateType, VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET)
        require_not_equal(info.descriptorSetLayout, 0)
        require_equal(info.pipelineBindPoint, 0)
        require_equal(info.pipelineLayout, 0)
        require_equal(info.set, 0)

        # The 1st entry
        entry = get_entry(create_template, arch, info, 0)
        require_equal(entry.dstBinding, 0)
        require_equal(entry.dstArrayElement, 0)
        require_equal(entry.descriptorCount, 3)
        require_equal(entry.descriptorType, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        require_equal(entry.offset, 0)
        require_equal(entry.stride, 24)
        # The 2nd entry
        entry = get_entry(create_template, arch, info, 1)
        require_equal(entry.dstBinding, 0)
        require_equal(entry.dstArrayElement, 5)
        require_equal(entry.descriptorCount, 1)
        require_equal(entry.descriptorType, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        require_equal(entry.offset, 3 * 24)
        require_equal(entry.stride, 24)
        # The 3rd entry
        entry = get_entry(create_template, arch, info, 2)
        require_equal(entry.dstBinding, 2)
        require_equal(entry.dstArrayElement, 1)
        require_equal(entry.descriptorCount, 2)
        require_equal(entry.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        require_equal(entry.offset, 256)
        require_equal(entry.stride, 24)
        # The 4th entry
        entry = get_entry(create_template, arch, info, 3)
        require_equal(entry.dstBinding, 2)
        require_equal(entry.dstArrayElement, 0)
        require_equal(entry.descriptorCount, 1)
        require_equal(entry.descriptorType, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        require_equal(entry.offset, 512)
        require_equal(entry.stride, 24)

        check_destroy_descriptor_update_template(self, device, template)
