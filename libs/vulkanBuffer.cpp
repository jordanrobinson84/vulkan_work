// #ifndef STB_IMAGE_IMPLEMENTATION
//     #define STB_IMAGE_IMPLEMENTATION
//     #include <stb/stb_image.h>
// #endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include <stb/stb_image_write.h>
#endif
#include <cassert>
#include <regex>
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
    if(data != nullptr){
        copyHostData(data, 0, memoryRequirements.size);
    }
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

    delete copyCommandBuffer;
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

///////////////////////////////////////
// VulkanImage
///////////////////////////////////////

bool VulkanImage::isRGBAOrder(VkFormat format){
    bool isRGBA = false;

    switch (format){
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            isRGBA = true;
            break;
        ////////////////////////////////////////
        // TODO: Block formats are special
        //
        // VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131,
        // VK_FORMAT_BC1_RGB_SRGB_BLOCK = 132,
        // VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
        // VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 134,
        // VK_FORMAT_BC2_UNORM_BLOCK = 135,
        // VK_FORMAT_BC2_SRGB_BLOCK = 136,
        // VK_FORMAT_BC3_UNORM_BLOCK = 137,
        // VK_FORMAT_BC3_SRGB_BLOCK = 138,
        // VK_FORMAT_BC4_UNORM_BLOCK = 139,
        // VK_FORMAT_BC4_SNORM_BLOCK = 140,
        // VK_FORMAT_BC5_UNORM_BLOCK = 141,
        // VK_FORMAT_BC5_SNORM_BLOCK = 142,
        // VK_FORMAT_BC6H_UFLOAT_BLOCK = 143,
        // VK_FORMAT_BC6H_SFLOAT_BLOCK = 144,
        // VK_FORMAT_BC7_UNORM_BLOCK = 145,
        // VK_FORMAT_BC7_SRGB_BLOCK = 146,
        // VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147,
        // VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148,
        // VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149,
        // VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150,
        // VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 151,
        // VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152,
        // VK_FORMAT_EAC_R11_UNORM_BLOCK = 153,
        // VK_FORMAT_EAC_R11_SNORM_BLOCK = 154,
        // VK_FORMAT_EAC_R11G11_UNORM_BLOCK = 155,
        // VK_FORMAT_EAC_R11G11_SNORM_BLOCK = 156,
        // VK_FORMAT_ASTC_4x4_UNORM_BLOCK = 157,
        // VK_FORMAT_ASTC_4x4_SRGB_BLOCK = 158,
        // VK_FORMAT_ASTC_5x4_UNORM_BLOCK = 159,
        // VK_FORMAT_ASTC_5x4_SRGB_BLOCK = 160,
        // VK_FORMAT_ASTC_5x5_UNORM_BLOCK = 161,
        // VK_FORMAT_ASTC_5x5_SRGB_BLOCK = 162,
        // VK_FORMAT_ASTC_6x5_UNORM_BLOCK = 163,
        // VK_FORMAT_ASTC_6x5_SRGB_BLOCK = 164,
        // VK_FORMAT_ASTC_6x6_UNORM_BLOCK = 165,
        // VK_FORMAT_ASTC_6x6_SRGB_BLOCK = 166,
        // VK_FORMAT_ASTC_8x5_UNORM_BLOCK = 167,
        // VK_FORMAT_ASTC_8x5_SRGB_BLOCK = 168,
        // VK_FORMAT_ASTC_8x6_UNORM_BLOCK = 169,
        // VK_FORMAT_ASTC_8x6_SRGB_BLOCK = 170,
        // VK_FORMAT_ASTC_8x8_UNORM_BLOCK = 171,
        // VK_FORMAT_ASTC_8x8_SRGB_BLOCK = 172,
        // VK_FORMAT_ASTC_10x5_UNORM_BLOCK = 173,
        // VK_FORMAT_ASTC_10x5_SRGB_BLOCK = 174,
        // VK_FORMAT_ASTC_10x6_UNORM_BLOCK = 175,
        // VK_FORMAT_ASTC_10x6_SRGB_BLOCK = 176,
        // VK_FORMAT_ASTC_10x8_UNORM_BLOCK = 177,
        // VK_FORMAT_ASTC_10x8_SRGB_BLOCK = 178,
        // VK_FORMAT_ASTC_10x10_UNORM_BLOCK = 179,
        // VK_FORMAT_ASTC_10x10_SRGB_BLOCK = 180,
        // VK_FORMAT_ASTC_12x10_UNORM_BLOCK = 181,
        // VK_FORMAT_ASTC_12x10_SRGB_BLOCK = 182,
        // VK_FORMAT_ASTC_12x12_UNORM_BLOCK = 183,
        // VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,
        default:
            isRGBA = false;
            break;
    }

    return isRGBA;
}

uint32_t VulkanImage::bytesPerPixel(VkFormat format){
    uint32_t BPP = 0;

    switch (format){
        case VK_FORMAT_UNDEFINED:
            BPP = 0;
            break;
        case VK_FORMAT_R4G4_UNORM_PACK8:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_S8_UINT:
            BPP = 1;
            break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
            BPP = 2;
            break;
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_D16_UNORM_S8_UINT:
            BPP = 3;
            break;
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            BPP = 4;
            break;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            BPP = 5;
            break;
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            BPP = 6;
            break;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
            BPP = 8;
            break;
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            BPP = 12;
            break;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
            BPP = 16;
            break;
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            BPP = 24;
            break;
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            BPP = 32;
            break;
        ////////////////////////////////////////
        // TODO: Block formats are special
        //
        // VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131,
        // VK_FORMAT_BC1_RGB_SRGB_BLOCK = 132,
        // VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
        // VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 134,
        // VK_FORMAT_BC2_UNORM_BLOCK = 135,
        // VK_FORMAT_BC2_SRGB_BLOCK = 136,
        // VK_FORMAT_BC3_UNORM_BLOCK = 137,
        // VK_FORMAT_BC3_SRGB_BLOCK = 138,
        // VK_FORMAT_BC4_UNORM_BLOCK = 139,
        // VK_FORMAT_BC4_SNORM_BLOCK = 140,
        // VK_FORMAT_BC5_UNORM_BLOCK = 141,
        // VK_FORMAT_BC5_SNORM_BLOCK = 142,
        // VK_FORMAT_BC6H_UFLOAT_BLOCK = 143,
        // VK_FORMAT_BC6H_SFLOAT_BLOCK = 144,
        // VK_FORMAT_BC7_UNORM_BLOCK = 145,
        // VK_FORMAT_BC7_SRGB_BLOCK = 146,
        // VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147,
        // VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148,
        // VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149,
        // VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150,
        // VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 151,
        // VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152,
        // VK_FORMAT_EAC_R11_UNORM_BLOCK = 153,
        // VK_FORMAT_EAC_R11_SNORM_BLOCK = 154,
        // VK_FORMAT_EAC_R11G11_UNORM_BLOCK = 155,
        // VK_FORMAT_EAC_R11G11_SNORM_BLOCK = 156,
        // VK_FORMAT_ASTC_4x4_UNORM_BLOCK = 157,
        // VK_FORMAT_ASTC_4x4_SRGB_BLOCK = 158,
        // VK_FORMAT_ASTC_5x4_UNORM_BLOCK = 159,
        // VK_FORMAT_ASTC_5x4_SRGB_BLOCK = 160,
        // VK_FORMAT_ASTC_5x5_UNORM_BLOCK = 161,
        // VK_FORMAT_ASTC_5x5_SRGB_BLOCK = 162,
        // VK_FORMAT_ASTC_6x5_UNORM_BLOCK = 163,
        // VK_FORMAT_ASTC_6x5_SRGB_BLOCK = 164,
        // VK_FORMAT_ASTC_6x6_UNORM_BLOCK = 165,
        // VK_FORMAT_ASTC_6x6_SRGB_BLOCK = 166,
        // VK_FORMAT_ASTC_8x5_UNORM_BLOCK = 167,
        // VK_FORMAT_ASTC_8x5_SRGB_BLOCK = 168,
        // VK_FORMAT_ASTC_8x6_UNORM_BLOCK = 169,
        // VK_FORMAT_ASTC_8x6_SRGB_BLOCK = 170,
        // VK_FORMAT_ASTC_8x8_UNORM_BLOCK = 171,
        // VK_FORMAT_ASTC_8x8_SRGB_BLOCK = 172,
        // VK_FORMAT_ASTC_10x5_UNORM_BLOCK = 173,
        // VK_FORMAT_ASTC_10x5_SRGB_BLOCK = 174,
        // VK_FORMAT_ASTC_10x6_UNORM_BLOCK = 175,
        // VK_FORMAT_ASTC_10x6_SRGB_BLOCK = 176,
        // VK_FORMAT_ASTC_10x8_UNORM_BLOCK = 177,
        // VK_FORMAT_ASTC_10x8_SRGB_BLOCK = 178,
        // VK_FORMAT_ASTC_10x10_UNORM_BLOCK = 179,
        // VK_FORMAT_ASTC_10x10_SRGB_BLOCK = 180,
        // VK_FORMAT_ASTC_12x10_UNORM_BLOCK = 181,
        // VK_FORMAT_ASTC_12x10_SRGB_BLOCK = 182,
        // VK_FORMAT_ASTC_12x12_UNORM_BLOCK = 183,
        // VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,
        default:
            BPP = 0;
            break;
    }

    assert(BPP != 0);

    return BPP;
}

VulkanImage::VulkanImage(VulkanDevice * __deviceContext,
                        VkImage __imageHandle,
                        VkImageUsageFlags __usage,
                        VkImageType __imageType,
                        VkFormat __format, 
                        VkExtent3D __extent,
                        VkImageCreateFlags __flags,
                        VkSampleCountFlagBits __sampleCount,
                        VkImageTiling __tiling,
                        uint32_t __mipLevels,
                        uint32_t __arrayLayers,
                        VkSharingMode __sharingMode,
                        uint32_t __queueFamilyIndexCount,
                        const uint32_t * __pQueueFamilyIndices,
                        VkImageLayout __layout){
    // Set context
    deviceContext   = __deviceContext;
    layout          = __layout;
    imageHandle     = __imageHandle;
    externalImage   = true;
    imageViewHandle = VK_NULL_HANDLE;

    // Fill in relevant fields
    imageCreateInfo.sType                   = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                   = nullptr;
    imageCreateInfo.flags                   = __flags;
    imageCreateInfo.imageType               = __imageType;
    imageCreateInfo.format                  = __format;
    imageCreateInfo.extent                  = __extent;
    imageCreateInfo.mipLevels               = __mipLevels;
    imageCreateInfo.arrayLayers             = __arrayLayers;
    imageCreateInfo.samples                 = __sampleCount;
    imageCreateInfo.tiling                  = __tiling;
    imageCreateInfo.usage                   = __usage;
    imageCreateInfo.sharingMode             = __sharingMode;
    imageCreateInfo.queueFamilyIndexCount   = __queueFamilyIndexCount;
    imageCreateInfo.pQueueFamilyIndices     = __pQueueFamilyIndices;
    imageCreateInfo.initialLayout           = __layout;
}

VulkanImage::VulkanImage(VulkanDevice * __deviceContext,
                VkImageUsageFlags __usage,
                VkImageType __imageType,
                VkFormat __format,
                VkExtent3D __extent,
                VkImageCreateFlags __flags,
                VkSampleCountFlagBits __sampleCount,
                VkImageTiling __tiling,
                uint32_t __mipLevels,
                uint32_t __arrayLayers,
                VkSharingMode __sharingMode,
                uint32_t __queueFamilyIndexCount,
                const uint32_t * __pQueueFamilyIndices,
                VkImageLayout __layout){
    // Set context
    deviceContext   = __deviceContext;
    layout          = __layout;
    externalImage   = false;
    imageViewHandle = VK_NULL_HANDLE;

    // Allocate and bind image
    imageCreateInfo.sType                   = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext                   = nullptr;
    imageCreateInfo.flags                   = __flags;
    imageCreateInfo.imageType               = __imageType;
    imageCreateInfo.format                  = __format;
    imageCreateInfo.extent                  = __extent;
    imageCreateInfo.mipLevels               = __mipLevels;
    imageCreateInfo.arrayLayers             = __arrayLayers;
    imageCreateInfo.samples                 = __sampleCount;
    imageCreateInfo.tiling                  = __tiling;
    imageCreateInfo.usage                   = __usage;
    imageCreateInfo.sharingMode             = __sharingMode;
    imageCreateInfo.queueFamilyIndexCount   = __queueFamilyIndexCount;
    imageCreateInfo.pQueueFamilyIndices     = __pQueueFamilyIndices;
    imageCreateInfo.initialLayout           = __layout;

    assert(deviceContext->vkCreateImage(deviceContext->device, &imageCreateInfo, nullptr, &imageHandle) == VK_SUCCESS);
    imageMemory = deviceContext->allocateAndBindMemory(imageHandle, false);
}

VulkanImage::~VulkanImage(){
    // This should never happen, but just in case, we can't do cleanup
    assert(deviceContext != nullptr);

    // Clean up image view
    if(imageViewHandle != VK_NULL_HANDLE){
        deviceContext->vkDestroyImageView(deviceContext->device, imageViewHandle, nullptr);
    }

    // Clean up image view
    if(imageHandle != VK_NULL_HANDLE && !externalImage){
        deviceContext->vkDestroyImage(deviceContext->device, imageHandle, nullptr);
    }
}

VkImageView VulkanImage::createImageView(VkImageViewType __imageViewType, VkImageAspectFlags __imageAspect, uint32_t __baseMipLevel, uint32_t __baseArrayLayer){
    // Image View creation
    imageViewCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext               = nullptr;
    imageViewCreateInfo.flags               = 0;
    imageViewCreateInfo.image               = imageHandle;
    imageViewCreateInfo.viewType            = __imageViewType;
    imageViewCreateInfo.format              = imageCreateInfo.format;
    imageViewCreateInfo.components          = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    imageViewCreateInfo.subresourceRange    = {__imageAspect, __baseMipLevel, imageCreateInfo.mipLevels, __baseArrayLayer, imageCreateInfo.arrayLayers};

    assert(deviceContext->vkCreateImageView(deviceContext->device, &imageViewCreateInfo, nullptr, &imageViewHandle) == VK_SUCCESS);

    return imageViewHandle;
}

void VulkanImage::loadImageData(const void * data, const uint32_t dataSize, VkExtent3D copyExtent, VkImageSubresourceLayers copySubresource, VkOffset3D copyOffset){
    VulkanBuffer imageBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, data, dataSize, false);

    VkBufferImageCopy imageCopy;
    imageCopy.bufferOffset                      = 0;
    imageCopy.bufferRowLength                   = 0;
    imageCopy.bufferImageHeight                 = 0;
    imageCopy.imageSubresource                  = copySubresource;
    imageCopy.imageOffset                       = copyOffset;
    imageCopy.imageExtent                       = copyExtent;

    // Find copy-capable queue (should be any queue)
    uint32_t copyQueueFamily = deviceContext->getUsableDeviceQueueFamily(VK_QUEUE_TRANSFER_BIT);
    assert(copyQueueFamily != (std::numeric_limits<uint32_t>::max)());

    VulkanCommandPool * copyCommandPool = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, copyQueueFamily);
    assert(copyCommandPool != nullptr);
    VkCommandBuffer * copyCommandBuffer = copyCommandPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    assert(copyCommandBuffer != nullptr);

    // Record into command buffer
    VkCommandBufferBeginInfo cbBeginInfo;
    cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.pNext = nullptr;
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cbBeginInfo.pInheritanceInfo = nullptr; // Not a secondary command buffer

    VkImageLayout oldLayout = layout;
    deviceContext->vkBeginCommandBuffer(copyCommandBuffer[0], &cbBeginInfo);
    setImageLayout(copyCommandBuffer[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    deviceContext->vkCmdCopyBufferToImage(copyCommandBuffer[0], imageBuffer.bufferHandle, imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    // Can't transition to undefined or pre-initialized layouts
    if(oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && oldLayout != VK_IMAGE_LAYOUT_UNDEFINED){
        setImageLayout(copyCommandBuffer[0], oldLayout);
    }
    deviceContext->vkEndCommandBuffer(copyCommandBuffer[0]);

    // Dispatch
    VkQueue copyQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, copyQueueFamily, 0, &copyQueue);
    VkSubmitInfo copySubmitInfo;
    copySubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    copySubmitInfo.pNext                = nullptr;
    copySubmitInfo.waitSemaphoreCount   = 0; // No wait semaphores
    copySubmitInfo.pWaitSemaphores      = nullptr; // No wait semaphores
    copySubmitInfo.pWaitDstStageMask    = nullptr; // No wait semaphores
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

    copyCommandPool->freeCommandBuffers(1, &copyCommandBuffer);
    delete copyCommandPool;
}

void VulkanImage::saveImage(const std::string& imageFileName){
    uint32_t alignment          = 16;
    uint32_t bytesPerPixel      = VulkanImage::bytesPerPixel(imageCreateInfo.format);
    uint32_t destBytesPerPixel  = 4;
    uint32_t imageRowLength     = imageCreateInfo.extent.width;
    uint32_t dataSize           = imageRowLength * imageCreateInfo.extent.height * destBytesPerPixel;
    VulkanBuffer imageBuffer(deviceContext, VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, dataSize, true);

    VkBufferImageCopy imageCopy;
    imageCopy.bufferOffset                      = 0;
    imageCopy.bufferRowLength                   = imageRowLength; 
    imageCopy.bufferImageHeight                 = imageCreateInfo.extent.height;
    imageCopy.imageSubresource                  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    imageCopy.imageOffset                       = {0, 0, 0};
    imageCopy.imageExtent                       = imageCreateInfo.extent;

    // Find copy-capable queue (should be any queue)
    uint32_t copyQueueFamily = deviceContext->getUsableDeviceQueueFamily(VK_QUEUE_TRANSFER_BIT);
    assert(copyQueueFamily != (std::numeric_limits<uint32_t>::max)());

    VulkanCommandPool * copyCommandPool = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, copyQueueFamily);
    assert(copyCommandPool != nullptr);
    VkCommandBuffer * copyCommandBuffer = copyCommandPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    assert(copyCommandBuffer != nullptr);

    // Record into command buffer
    VkCommandBufferBeginInfo cbBeginInfo;
    cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbBeginInfo.pNext = nullptr;
    cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cbBeginInfo.pInheritanceInfo = nullptr; // Not a secondary command buffer

    // Blit image to an R(G)(B)(A) 8bpp target if the source image is not R(G)(B)(A) or 8bpp
    bool isRGBA = VulkanImage::isRGBAOrder(imageCreateInfo.format);
    bool isBlittable = (deviceContext->getSupportedFormat({imageCreateInfo.format}, imageCreateInfo.tiling, VK_FORMAT_FEATURE_BLIT_SRC_BIT ) == imageCreateInfo.format) && 
                       (imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT == VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    if(!isBlittable){
        std::cout << "The source image does not support blits, falling back to standard RAW copy-to-buffer" << std::endl;
    }

    // Choose first from unsigned formats, then switch to signed formats if unsigned formats don't support blitting
    std::vector<VkFormat> blitCandidateFormats = {VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_FORMAT_R8G8B8A8_UINT,
                                                  VK_FORMAT_R8G8B8A8_USCALED,
                                                  VK_FORMAT_R8G8B8A8_SRGB,
                                                  VK_FORMAT_R8G8B8A8_SNORM,
                                                  VK_FORMAT_R8G8B8A8_SINT,
                                                  VK_FORMAT_R8G8B8A8_SSCALED};

    VkFormat blitDstFormat = deviceContext->getSupportedFormat(blitCandidateFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT );
    assert(blitDstFormat != VK_FORMAT_UNDEFINED);
    VulkanImage blitImage(deviceContext, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TYPE_2D, blitDstFormat, imageCreateInfo.extent);
    blitImage.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    // Perform blit to an R8G8B8A8 Image for saving to an image file
    if((!isRGBA || bytesPerPixel != 4) && isBlittable){
        VkImageBlit blit;
        blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        blit.srcOffsets[0]  = { 0,0,0 };
        blit.srcOffsets[1]  = { (int32_t)imageCreateInfo.extent.width, (int32_t)imageCreateInfo.extent.height, 1};
        blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        blit.dstOffsets[0]  = { 0,0,0 };
        blit.dstOffsets[1]  = { (int32_t)imageCreateInfo.extent.width, (int32_t)imageCreateInfo.extent.height, 1};

        VkImageLayout oldLayout = layout;
        VkImageLayout blitOldLayout = blitImage.layout;
        deviceContext->vkBeginCommandBuffer(copyCommandBuffer[0], &cbBeginInfo);
        setImageLayout(copyCommandBuffer[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        blitImage.setImageLayout(copyCommandBuffer[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        deviceContext->vkCmdBlitImage(copyCommandBuffer[0], imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, blitImage.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
        // Can't transition to undefined or pre-initialized layouts
        if(oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && oldLayout != VK_IMAGE_LAYOUT_UNDEFINED){
            setImageLayout(copyCommandBuffer[0], oldLayout);
        }
        blitImage.setImageLayout(copyCommandBuffer[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        deviceContext->vkCmdCopyImageToBuffer(copyCommandBuffer[0], blitImage.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageBuffer.bufferHandle, 1, &imageCopy);

        // Can't transition to undefined or pre-initialized layouts
        if(blitOldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && blitOldLayout != VK_IMAGE_LAYOUT_UNDEFINED){
            blitImage.setImageLayout(copyCommandBuffer[0], blitOldLayout);
        }
        deviceContext->vkEndCommandBuffer(copyCommandBuffer[0]);
    }else{
        // Standard image copy
        VkImageLayout oldLayout = layout;
        deviceContext->vkBeginCommandBuffer(copyCommandBuffer[0], &cbBeginInfo);
        setImageLayout(copyCommandBuffer[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        deviceContext->vkCmdCopyImageToBuffer(copyCommandBuffer[0], imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageBuffer.bufferHandle, 1, &imageCopy);
        // Can't transition to undefined or pre-initialized layouts
        if(oldLayout != VK_IMAGE_LAYOUT_PREINITIALIZED && oldLayout != VK_IMAGE_LAYOUT_UNDEFINED){
            setImageLayout(copyCommandBuffer[0], oldLayout);
        }
        deviceContext->vkEndCommandBuffer(copyCommandBuffer[0]);
    }

    // Dispatch
    VkQueue copyQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, copyQueueFamily, 0, &copyQueue);
    VkSubmitInfo copySubmitInfo;
    copySubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    copySubmitInfo.pNext                = nullptr;
    copySubmitInfo.waitSemaphoreCount   = 0; // No wait semaphores
    copySubmitInfo.pWaitSemaphores      = nullptr; // No wait semaphores
    copySubmitInfo.pWaitDstStageMask    = nullptr; // No wait semaphores
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

    // Invalidate memory to make it visible to host
    VkMappedMemoryRange imageBufferMemoryRange;
    imageBufferMemoryRange.sType    = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    imageBufferMemoryRange.pNext    = nullptr;
    imageBufferMemoryRange.memory   = imageBuffer.bufferMemory;
    imageBufferMemoryRange.offset   = 0;
    imageBufferMemoryRange.size     = VK_WHOLE_SIZE;
    assert(deviceContext->vkInvalidateMappedMemoryRanges(deviceContext->device, 1, &imageBufferMemoryRange) == VK_SUCCESS);

    // Save image
    void * bufferData = nullptr;
    assert( deviceContext->vkMapMemory(deviceContext->device, imageBuffer.bufferMemory, 0, dataSize, 0, &bufferData) == VK_SUCCESS);
    assert(bufferData != nullptr);

    std::vector<char> copyArray(dataSize, ' ');
    memcpy(&copyArray[0], bufferData, dataSize);
    deviceContext->vkUnmapMemory(deviceContext->device, imageBuffer.bufferMemory);

    int w = (int)imageCreateInfo.extent.width;
    int h = (int)imageCreateInfo.extent.height;

    // Choose the appropriate save function
    std::regex extensionRegex(".+\\.(.+)");
    std::smatch matches;
    std::string extension;

    if(std::regex_match(imageFileName, matches, extensionRegex)){
        try{
            extension = (matches[1].str)();
        }catch(std::out_of_range oor){
            std::cout << "Invalid extension " << extension << std::endl;
            extension = "raw";
        }catch(std::invalid_argument ia){
            std::cout << "Invalid extension " << extension << std::endl;
            extension = "raw";
        }
    }

    if(isBlittable){
        if(extension.compare("png") == 0){
            stbi_write_png(imageFileName.c_str(), w, h, 4, &copyArray[0], imageRowLength * destBytesPerPixel);
        }else if(extension.compare("bmp") == 0){
            stbi_write_bmp(imageFileName.c_str(), w, h, 4, &copyArray[0]);
        }else if(extension.compare("tga") == 0){
            stbi_write_tga(imageFileName.c_str(), w, h, 4, &copyArray[0]);
        }else if(extension.compare("jpg") == 0){
            int jpegQuality = 75;
            stbi_write_jpg(imageFileName.c_str(), w, h, 4, &copyArray[0], jpegQuality);
        }else if(extension.compare("hdr") == 0){
            stbi_write_hdr(imageFileName.c_str(), w, h, 4, (const float *)&copyArray[0]);
        }else{
            // Raw dump
            std::ofstream rawImageStream(imageFileName.c_str(), std::ofstream::binary);
            rawImageStream.write(&copyArray[0], dataSize);
            rawImageStream.close();
        }
    }else{
        // Raw dump
        std::ofstream rawImageStream(imageFileName.c_str(), std::ofstream::binary);
        rawImageStream.write(&copyArray[0], dataSize);
        rawImageStream.close();
    }

    copyArray.clear();

    copyCommandPool->freeCommandBuffers(1, &copyCommandBuffer);
    delete copyCommandBuffer;
    delete copyCommandPool;
}

void VulkanImage::setImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout){
    assert(imageViewHandle != VK_NULL_HANDLE);

    VkImageMemoryBarrier imageBarrier;
    imageBarrier.sType                  = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext                  = nullptr;
    imageBarrier.srcAccessMask          = 0;
    imageBarrier.dstAccessMask          = 0;
    imageBarrier.oldLayout              = layout;
    imageBarrier.newLayout              = newLayout;
    imageBarrier.srcQueueFamilyIndex    = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex    = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image                  = imageHandle;
    imageBarrier.subresourceRange       = imageViewCreateInfo.subresourceRange;

    VkPipelineStageFlags srcStageMask = (VkPipelineStageFlags)0;
    VkPipelineStageFlags dstStageMask = (VkPipelineStageFlags)0;

    switch (layout) {
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageBarrier.srcAccessMask =
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                           VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                           VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                           VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        default:
            break;
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            srcStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageBarrier.dstAccessMask |=
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStageMask |= (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageBarrier.srcAccessMask =
                VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                           VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
                           VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
                           VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
        default:
            break;
    }

    srcStageMask = (srcStageMask == 0) ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : srcStageMask;
    dstStageMask = (dstStageMask == 0) ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : dstStageMask;

    deviceContext->vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    layout = newLayout;
}