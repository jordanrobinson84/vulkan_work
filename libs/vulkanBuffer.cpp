#include <cassert>
#include "vulkanBuffer.h"

std::shared_ptr<VulkanCommandPool> VulkanBuffer::copyCommandPool;

VulkanBuffer::VulkanBuffer(VulkanDevice * __deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool __hostVisible ){

    // Set the Device Context
    deviceContext = __deviceContext;

    hostVisible = __hostVisible;

    // Buffer Creation Info
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage | (hostVisible ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0; // Not sharing
    bufferInfo.pQueueFamilyIndices = nullptr;

    assert(deviceContext->vkCreateBuffer(deviceContext->device, &bufferInfo, nullptr, &bufferHandle) == VK_SUCCESS);

    // Get Memory Requirements
    payloadSize = dataSize;
    deviceContext->vkGetBufferMemoryRequirements(deviceContext->device, bufferHandle, &memoryRequirements);
    std::cout << "Memory requirements for " << (hostVisible ? "Host " : "Device ") << "buffer: " << std::dec << memoryRequirements.size << std::endl;
    uint32_t memoryType = deviceContext->getUsableMemoryType(memoryRequirements.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if ( memoryType == (std::numeric_limits<uint32_t>::max)()){
        memoryType = deviceContext->getUsableMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        assert (memoryType != (std::numeric_limits<uint32_t>::max)());
    }
    std::cout << "Memory type for  " << (hostVisible ? "Host " : "Device ") << "buffer: " << std::dec << memoryType << std::endl;

    // Allocate Memory
    bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferAllocateInfo.pNext = nullptr;
    bufferAllocateInfo.allocationSize = memoryRequirements.size;
    bufferAllocateInfo.memoryTypeIndex = memoryType;
    assert( deviceContext->vkAllocateMemory(deviceContext->device, &bufferAllocateInfo, nullptr, &bufferMemory) == VK_SUCCESS);

    // Bind Memory for buffer
    assert( deviceContext->vkBindBufferMemory(deviceContext->device, bufferHandle, bufferMemory, 0) == VK_SUCCESS);

    // Copy data to buffer
    copyHostData(data, 0, memoryRequirements.size);
}

VulkanBuffer::~VulkanBuffer(){
    // Destroy Buffer
    deviceContext->vkDestroyBuffer(deviceContext->device, bufferHandle, nullptr);

    // Free Memory
    deviceContext->vkFreeMemory(deviceContext->device, bufferMemory, nullptr);
}

void VulkanBuffer::copyBuffer(const VulkanBuffer& srcBuffer, uint32_t offset, uint32_t dataSize){

    // Copy between buffers
    VkBufferCopy bufferToBufferCopy;
    bufferToBufferCopy.srcOffset    = 0;
    bufferToBufferCopy.dstOffset    = offset;
    bufferToBufferCopy.size         = dataSize;

    // Find copy-capable queue (should be any queue)
    uint32_t copyQueueFamily = deviceContext->getUsableDeviceQueueFamily(VK_QUEUE_TRANSFER_BIT);
    assert(copyQueueFamily != (std::numeric_limits<uint32_t>::max)());

    if(VulkanBuffer::copyCommandPool == nullptr){
        VulkanBuffer::copyCommandPool.reset(deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, copyQueueFamily));
    }
    assert(copyCommandPool != nullptr);
    VkCommandBuffer * copyCommandBuffer = copyCommandPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    assert(copyCommandBuffer != nullptr);

    // Record into command buffer
    VkCommandBufferBeginInfo cbBeginInfo;
    cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.pNext = nullptr;
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cbBeginInfo.pInheritanceInfo = nullptr; // Not a secondary command buffer

    deviceContext->vkBeginCommandBuffer(copyCommandBuffer[0], &cbBeginInfo);
    deviceContext->vkCmdCopyBuffer(copyCommandBuffer[0], srcBuffer.bufferHandle, bufferHandle, 1, &bufferToBufferCopy);
    deviceContext->vkEndCommandBuffer(copyCommandBuffer[0]);

    // Dispatch
    VkQueue copyQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, copyQueueFamily, 0, &copyQueue);
    VkSubmitInfo copySubmitInfo;
    copySubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    copySubmitInfo.pNext                = nullptr;
    copySubmitInfo.waitSemaphoreCount   = 0; // No wait semaphores
    copySubmitInfo.pWaitSemaphores      = nullptr; // No wait semaphores
    copySubmitInfo.pWaitDstStageMask    =   nullptr; // No wait semaphores
    copySubmitInfo.commandBufferCount   = 1;
    copySubmitInfo.pCommandBuffers      = copyCommandBuffer;
    copySubmitInfo.signalSemaphoreCount = 0; // No signal semaphores
    copySubmitInfo.pSignalSemaphores    = nullptr; // No signal semaphores

    // Fence
    VkFence copyFence;
    VkFenceCreateInfo copyFenceInfo;
    copyFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    copyFenceInfo.pNext = nullptr;
    copyFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    assert(deviceContext->vkCreateFence(deviceContext->device, &copyFenceInfo, nullptr, &copyFence) == VK_SUCCESS);
    assert(deviceContext->vkResetFences(deviceContext->device, 1, &copyFence) == VK_SUCCESS);

    assert(deviceContext->vkQueueSubmit(copyQueue, 1, &copySubmitInfo, copyFence) == VK_SUCCESS);
    assert(deviceContext->vkWaitForFences(deviceContext->device, 1, &copyFence, VK_TRUE, 0x40000000) == VK_SUCCESS);
    deviceContext->vkDestroyFence(deviceContext->device, copyFence, nullptr);

    copyCommandPool.reset();
}

void VulkanBuffer::copyHostData(const void * data, uint32_t offset, uint32_t dataSize){
    // Offset must be at least DWORD aligned
    if(offset % 4 != 0){
        std::runtime_error("Copy offset must be at least DWORD-aligned!");
    }

    // Host-visible
    if(hostVisible){
        // Set buffer data
        void * bufferData = nullptr;
        assert( deviceContext->vkMapMemory(deviceContext->device, bufferMemory, offset, dataSize, 0, &bufferData) == VK_SUCCESS);
        assert(bufferData != nullptr);
        memcpy(bufferData, data, dataSize);

        // Flush to make data GPU-visible
        VkMappedMemoryRange hostMemoryRange;
        hostMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        hostMemoryRange.pNext = nullptr;
        hostMemoryRange.memory = bufferMemory;
        hostMemoryRange.offset = 0;
        hostMemoryRange.size = memoryRequirements.size;
        deviceContext->vkFlushMappedMemoryRanges(deviceContext->device, 1, &hostMemoryRange);
        deviceContext->vkUnmapMemory(deviceContext->device, bufferMemory);
    }else{ // Create copy buffer
        // Source Buffer must be host-mapped
        VulkanBuffer * srcBuffer = new VulkanBuffer(deviceContext, bufferInfo.usage, data, dataSize, true);
        copyBuffer(*srcBuffer, offset, dataSize);
        delete srcBuffer;
    }
}