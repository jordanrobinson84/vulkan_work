#ifndef __VULKAN_BUFFER_H__
#define __VULKAN_BUFFER_H__

// #ifndef STB_IMAGE_IMPLEMENTATION
//     #define STB_IMAGE_IMPLEMENTATION
//     #include <stb/stb_image.h>
// #endif
// #ifndef STB_IMAGE_WRITE_IMPLEMENTATION
//     #define STB_IMAGE_WRITE_IMPLEMENTATION
//     #include <stb/stb_image_write.h>
// #endif
#include <cstring>
#include "VulkanCommandPool.h"
#include "VulkanDriverInstance.h"

class VulkanBuffer{
private:

public:
    VulkanBuffer(VulkanDevice * __deviceContext, VkBufferUsageFlags usage, const void * data, const uint32_t dataSize, const bool __hostVisible );
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

class VulkanImage{
private:
    bool                        externalImage;
    VkImageCreateInfo           imageCreateInfo;

public:
    // Create wrapper class instance from existing VkImage
    VulkanImage(VulkanDevice * __deviceContext,
                VkImage __imageHandle,
                VkImageUsageFlags __usage,
                VkImageType __imageType,
                VkFormat __format,
                VkExtent3D __extent, 
                VkImageCreateFlags __flags              = 0,
                VkSampleCountFlagBits __sampleCount     = VK_SAMPLE_COUNT_1_BIT,
                VkImageTiling __tiling                  = VK_IMAGE_TILING_OPTIMAL,
                uint32_t __mipLevels                    = 1,
                uint32_t __arrayLayers                  = 1,
                VkSharingMode __sharingMode             = VK_SHARING_MODE_EXCLUSIVE,
                uint32_t __queueFamilyIndexCount        = 0,
                const uint32_t * __pQueueFamilyIndices  = nullptr,
                VkImageLayout __layout                  = VK_IMAGE_LAYOUT_UNDEFINED);
    VulkanImage(VulkanDevice * __deviceContext,
                VkImageUsageFlags __usage,
                VkImageType __imageType,
                VkFormat __format,
                VkExtent3D __extent,
                VkImageCreateFlags __flags              = 0,
                VkSampleCountFlagBits __sampleCount     = VK_SAMPLE_COUNT_1_BIT,
                VkImageTiling __tiling                  = VK_IMAGE_TILING_OPTIMAL,
                uint32_t __mipLevels                    = 1,
                uint32_t __arrayLayers                  = 1,
                VkSharingMode __sharingMode             = VK_SHARING_MODE_EXCLUSIVE,
                uint32_t __queueFamilyIndexCount        = 0,
                const uint32_t * __pQueueFamilyIndices  = nullptr,
                VkImageLayout __layout                  = VK_IMAGE_LAYOUT_UNDEFINED);
    ~VulkanImage();

    static bool isRGBAOrder(VkFormat format);
    static uint32_t bytesPerPixel(VkFormat format);
    void blitImage(VkCommandBuffer commandBuffer, VulkanImage& destImage, VkImageBlit *blitPtr = nullptr, VkFilter filter = VK_FILTER_LINEAR);
    VkImageView createImageView(VkImageViewType __imageViewType, VkImageAspectFlags __imageAspect, uint32_t __baseMipLevel = 0, uint32_t __baseArrayLayer = 0);
    void copyImageToBuffer(VkCommandBuffer commandBuffer, VulkanBuffer& destBuffer, VkBufferImageCopy *imageCopyPtr = nullptr);
    const VkImageCreateInfo& getImageCreateInfo(){return imageCreateInfo;};
    void loadImageData(const void * data, const uint32_t dataSize, VkExtent3D copyExtent, VkImageSubresourceLayers copySubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, VkOffset3D copyOffset = {0, 0, 0});
    void saveImage(const std::string& imageFileName);
    void setImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

    VulkanDevice *              deviceContext;
    VkImage                     imageHandle;
    VkDeviceMemory              imageMemory;
    VkImageView                 imageViewHandle;
    VkImageViewCreateInfo       imageViewCreateInfo;
    VkImageLayout               layout;
};

#endif