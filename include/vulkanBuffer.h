#ifndef __VULKAN_BUFFER_H__
#define __VULKAN_BUFFER_H__

#include <cstring>
#include "vulkanCommandPool.h"
#include "vulkanDriverInstance.h"

class VulkanBuffer{
private:

public:
    VulkanBuffer(VulkanDevice * deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool __hostVisible );
    ~VulkanBuffer();
    void copyBuffer(const VulkanBuffer& srcBuffer, uint32_t offset, uint32_t dataSize);
    void copyHostData(const void * data, uint32_t offset, uint32_t size);

    VkBuffer bufferHandle;
    VkBufferCreateInfo bufferInfo;
    VulkanDevice * deviceContext;
    VkDeviceMemory bufferMemory;
    VkMemoryAllocateInfo bufferAllocateInfo;
    bool hostVisible;
    VkMemoryRequirements memoryRequirements;
    uint32_t payloadSize;
    static std::shared_ptr<VulkanCommandPool> copyCommandPool; 
};

#endif