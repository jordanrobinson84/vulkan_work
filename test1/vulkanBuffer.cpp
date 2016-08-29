#include <cassert>
#include "vulkanBuffer.h"

VulkanBuffer::VulkanBuffer(const VulkanDevice * __deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool hostVisible ){

    // Set the Device Context
    deviceContext = __deviceContext;

    // Buffer Creation Info
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0;
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage | (hostVisible ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0; // Not sharing
    bufferInfo.pQueueFamilyIndices = nullptr;

    assert(deviceContext->vkCreateBuffer(deviceContext->device, &bufferInfo, nullptr, &buffer) == VK_SUCCESS);

    // // Buffer View Creation Info
    // VkBufferViewCreateInfo bufferViewInfo;
    // bufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    // bufferViewInfo.pNext = nullptr;
    // bufferViewInfo.flags = 0;
    // bufferViewInfo.buffer = *buffer;
    // bufferViewInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
    // bufferViewInfo.offset = 0;
    // bufferViewInfo.range = VK_WHOLE_SIZE; // Use the entire buffer
    // assert(deviceContext->vkCreateBufferView(deviceContext->device, &bufferViewInfo, nullptr, bufferView) == VK_SUCCESS);

    // Get Memory Requirements
    deviceContext->vkGetBufferMemoryRequirements(deviceContext->device, buffer, &memoryRequirements);
    std::cout << "Memory requirements for vertex buffer: " << std::dec << memoryRequirements.size << std::endl;
    int32_t memoryType = deviceContext->getUsableMemoryType(memoryRequirements.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if ( memoryType == -1){
        memoryType = deviceContext->getUsableMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        assert (memoryType != -1);
    }
    std::cout << "Memory type for vertex buffer: " << std::dec << memoryType << std::endl;

    // Allocate Memory
    bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferAllocateInfo.pNext = nullptr;
    bufferAllocateInfo.allocationSize = memoryRequirements.size;
    bufferAllocateInfo.memoryTypeIndex = memoryType;
    assert( deviceContext->vkAllocateMemory(deviceContext->device, &bufferAllocateInfo, nullptr, &bufferMemory) == VK_SUCCESS);

    // Bind Memory for buffer
    assert( deviceContext->vkBindBufferMemory(deviceContext->device, bufferHandle, bufferMemory, 0) == VK_SUCCESS);

    if (hostVisible){
        // Set buffer data
        void * bufferData = nullptr;
        assert( deviceContext->vkMapMemory(deviceContext->device, bufferMemory, 0, memoryRequirements.size, 0, &bufferData) == VK_SUCCESS);
        assert(bufferData != nullptr);
        memcpy(bufferData, data, dataSize);
        deviceContext->vkUnmapMemory(deviceContext->device, bufferMemory);
    
        // Flush to make data GPU-visible
        VkMappedMemoryRange hostMemoryRange;
        hostMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        hostMemoryRange.pNext = nullptr;
        hostMemoryRange.memory = bufferMemory;
        hostMemoryRange.offset = 0;
        hostMemoryRange.size = memoryRequirements.size;
        deviceContext->vkFlushMappedMemoryRanges(deviceContext->device, 1, &hostMemoryRange);

    // Copy from host-mapped buffer to device local buffer
    }else{

        // Source Buffer must be host-mapped
        VulkanBuffer * srcBuffer(deviceContext, usage, data, dataSize, true);

        // Copy between buffers
        VkBufferCopy hostToDeviceCopy;
        hostToDeviceCopy.srcOffset = 0;
        hostToDeviceCopy.dstOffset = 0;
        hostToDeviceCopy.size = memoryRequirements.size;

        // Find copy-capable queue (should be any queue)
        int32_t copyQueueFamily = deviceContext->getUsableDeviceQueue(VK_QUEUE_TRANSFER_BIT);
        assert(copyQueueFamily != -1);

        VulkanCommandPool * copyCommandPool = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, copyQueueFamily);
        assert(copyCommandPool != nullptr);
        VkCommandBuffer * copyCommandBuffer = copyCommandPool->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        assert(copyCommandBuffer != nullptr);

        // (TODO)Record into command buffer

        deviceContext->vkCmdCopyBuffer(commandBuffer, srcBuffer->bufferhandle, buffer, 1, &hostToDeviceCopy);

        delete srcBuffer;
    }
}

VulkanBuffer::~VulkanBuffer(){
    // Destroy Buffer
    deviceContext->vkDestroyBuffer(deviceContext->device, bufferHandle, nullptr);

    // Free Memory
    deviceContext->vkFreeMemory(deviceContext->device, bufferMemory, nullptr);
}