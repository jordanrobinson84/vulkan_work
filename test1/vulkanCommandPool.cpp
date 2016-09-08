#include "vulkanCommandPool.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice * __deviceContext, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex){
    deviceContext = __deviceContext;

    // Set up the Command Pool
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    info.queueFamilyIndex = queueFamilyIndex;
    assert(deviceContext->vkCreateCommandPool(deviceContext->device, &info, nullptr, &commandPoolHandle) == VK_SUCCESS);
}

VulkanCommandPool::~VulkanCommandPool(){
    deviceContext->vkDestroyCommandPool(deviceContext->device, commandPoolHandle, nullptr);
}

VkCommandBuffer * VulkanCommandPool::getCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount){
    VkCommandBuffer * commandBufferArray = new VkCommandBuffer[commandBufferCount];

    VkCommandBufferAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = commandPoolHandle;
    info.level = level;
    info.commandBufferCount = commandBufferCount;
    assert(deviceContext->vkAllocateCommandBuffers(deviceContext->device, &info, commandBufferArray) == VK_SUCCESS);

    return commandBufferArray;
}

void VulkanCommandPool::freeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer * commandBuffers){
    // Delete and free memory
    deviceContext->vkFreeCommandBuffers(deviceContext->device, commandPoolHandle, commandBufferCount, commandBuffers);
    delete[] commandBuffers;
}

// void VulkanCommandPool::resetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags){
//     assert(deviceContext->vkResetCommandBuffer(commandBuffer, flags) == VK_SUCCESS);
// }

void VulkanCommandPool::resetCommandPool(VkCommandPoolResetFlags flags){
    assert(deviceContext->vkResetCommandPool(deviceContext->device, commandPoolHandle, flags) == VK_SUCCESS);
}