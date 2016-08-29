#include <cassert>
#include "vulkanBuffer.h"

VulkanBuffer::VulkanBuffer(VulkanDevice * deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool hostVisible ){

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
    assert( deviceContext->vkBindBufferMemory(deviceContext->device, buffer, bufferMemory, 0) == VK_SUCCESS);

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
        // Buffer Creation Info
        VkBuffer srcBuffer;
        VkBufferCreateInfo srcBufferInfo;
        srcBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        srcBufferInfo.pNext = nullptr;
        srcBufferInfo.flags = 0;
        srcBufferInfo.size = dataSize;
        srcBufferInfo.usage = usage | (hostVisible ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        srcBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        srcBufferInfo.queueFamilyIndexCount = 0; // Not sharing
        srcBufferInfo.pQueueFamilyIndices = nullptr;

        assert(deviceContext->vkCreateBuffer(deviceContext->device, &srcBufferInfo, nullptr, &srcBuffer) == VK_SUCCESS);

        // Get Memory Requirements
        VkMemoryRequirements srcMemoryRequirements;
        deviceContext->vkGetBufferMemoryRequirements(deviceContext->device, buffer, &srcMemoryRequirements);
        std::cout << "Memory requirements for vertex source buffer: " << std::dec << srcMemoryRequirements.size << std::endl;
        memoryType = deviceContext->getUsableMemoryType(srcMemoryRequirements.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if ( memoryType == -1){
            memoryType = deviceContext->getUsableMemoryType(srcMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            assert (memoryType != -1);
        }
        std::cout << "Memory type for vertex source buffer: " << std::dec << memoryType << std::endl;

        // Allocate Memory
        VkDeviceMemory srcBufferMemory;
        VkMemoryAllocateInfo srcBufferAllocateInfo;
        srcBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        srcBufferAllocateInfo.pNext = nullptr;
        srcBufferAllocateInfo.allocationSize = srcMemoryRequirements.size;
        srcBufferAllocateInfo.memoryTypeIndex = memoryType;
        assert( deviceContext->vkAllocateMemory(deviceContext->device, &srcBufferAllocateInfo, nullptr, &srcBufferMemory) == VK_SUCCESS);

        // Bind Memory for srcBuffer
        assert( deviceContext->vkBindBufferMemory(deviceContext->device, srcBuffer, srcBufferMemory, 0) == VK_SUCCESS);

        // Set srcBuffer data
        void * srcBufferData = nullptr;
        assert( deviceContext->vkMapMemory(deviceContext->device, srcBufferMemory, 0, srcMemoryRequirements.size, 0, &srcBufferData) == VK_SUCCESS);
        assert(srcBufferData != nullptr);
        memcpy(srcBufferData, data, dataSize);
        deviceContext->vkUnmapMemory(deviceContext->device, srcBufferMemory);

        // Flush to make data GPU-visible
        VkMappedMemoryRange hostMemoryRange;
        hostMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        hostMemoryRange.pNext = nullptr;
        hostMemoryRange.memory = srcBufferMemory;
        hostMemoryRange.offset = 0;
        hostMemoryRange.size = srcMemoryRequirements.size;
        deviceContext->vkFlushMappedMemoryRanges(deviceContext->device, 1, &hostMemoryRange);

        // Copy between buffers
        // VkBufferCopy hostToDeviceCopy;
        // hostToDeviceCopy.srcOffset = 0;
        // hostToDeviceCopy.dstOffset;
        // hostToDeviceCopy.size = memoryRequirements.size;

        // deviceContext->vkCmdCopyBuffer(commandBuffer, srcBuffer, buffer, 1, &hostToDeviceCopy);
    }
}