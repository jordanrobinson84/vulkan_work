#ifndef __VULKAN_COMMAND_POOL_H__
#define __VULKAN_COMMAND_POOL_H__

#include "vulkanDriverInstance.h"

class VulkanCommandPool{
    VulkanCommandPool(const VulkanDevice * __deviceContext, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    VkCommandBuffer * getCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount);
    void freeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer * commandBuffers);
    void resetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
    void resetCommandPool(VkCommandPoolResetFlags flags);

    VkCommandPool commandPoolHandle;
    VulkanDevice * deviceContext;
};

#endif