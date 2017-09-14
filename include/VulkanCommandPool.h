#ifndef __VULKAN_COMMAND_POOL_H__
#define __VULKAN_COMMAND_POOL_H__

#include "VulkanDriverInstance.h"

struct VulkanDevice;

class VulkanCommandPool{
public:
    VulkanCommandPool(VulkanDevice * __deviceContext, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    ~VulkanCommandPool();
    VkCommandBuffer * getCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount);
    void freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer ** commandBuffers);
    void resetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
    void resetCommandPool(VkCommandPoolResetFlags flags);

    VkCommandPool commandPoolHandle;
    VulkanDevice * deviceContext;
};

#endif