#ifndef __VULKAN_BUFFER_H__
#define __VULKAN_BUFFER_H__

#include <cstring>
#include "vulkanDriverInstance.h"

class VulkanBuffer{
private:

public:
    VulkanBuffer(VulkanDevice * deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool hostVisible );

    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo;
    VkDeviceMemory bufferMemory;
    VkMemoryAllocateInfo bufferAllocateInfo;
    VkMemoryRequirements memoryRequirements;
};

#endif