#include "vulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanDevice * __deviceContext, VkPhysicalDevice physicalDevice, VkSurfaceKHR swapchainSurface, std::vector<uint32_t> & supportedQueueFamilyIndices){
    deviceContext       = __deviceContext;
    surface             = swapchainSurface;
    surfaceFormatIndex  = 0; // Default surface format
    swapchainImageIndex = 0;

    assert(deviceContext != nullptr);

    querySwapchain(physicalDevice);

    // Create presentation semaphore
    VkSemaphoreCreateInfo presentationSemaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    assert(deviceContext->vkCreateSemaphore(deviceContext->device, &presentationSemaphoreCreateInfo, nullptr, &presentationSemaphore) == VK_SUCCESS);

    // Create rendering done semaphore
    VkSemaphoreCreateInfo renderingDoneSemaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    assert(deviceContext->vkCreateSemaphore(deviceContext->device, &renderingDoneSemaphoreCreateInfo, nullptr, &renderingDoneSemaphore) == VK_SUCCESS);

    // Get surface image count
    uint32_t imageCount = surfaceCaps.minImageCount + 1;
    imageCount = (imageCount > surfaceCaps.maxImageCount) ? surfaceCaps.maxImageCount : imageCount;

    // Swapchain creation info
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext                   = nullptr;
    swapchainCreateInfo.flags                   = 0;
    swapchainCreateInfo.surface                 = surface;
    swapchainCreateInfo.minImageCount           = imageCount;
    swapchainCreateInfo.imageFormat             = surfaceFormats[surfaceFormatIndex].format;
    swapchainCreateInfo.imageColorSpace         = surfaceFormats[surfaceFormatIndex].colorSpace;
    swapchainCreateInfo.imageExtent             = extent;
    swapchainCreateInfo.imageArrayLayers        = 1;
    swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount   = supportedQueueFamilyIndices.size();
    swapchainCreateInfo.pQueueFamilyIndices     = &supportedQueueFamilyIndices[0];
    swapchainCreateInfo.preTransform            = surfaceCaps.currentTransform;
    swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode             = presentModes[0];
    swapchainCreateInfo.clipped                 = VK_TRUE;
    swapchainCreateInfo.oldSwapchain            = nullptr;

    assert(deviceContext->vkCreateSwapchainKHR(deviceContext->device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);

    // Get Swapchain images (access-controlled)
    uint32_t swapchainImageCount = 0;
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &swapchainImageCount, nullptr) == VK_SUCCESS);
    assert(swapchainImageCount != 0);
    swapchainImages = std::vector<VkImage>(swapchainImageCount);
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &swapchainImageCount, &swapchainImages[0]) == VK_SUCCESS);

    swapchainImageViews = std::vector<VkImageView>(imageCount);
    swapchainFramebuffers = std::vector<VkFramebuffer>(imageCount);

    for(uint32_t index = 0; index < imageCount; index++){
        std::cout << "Swapchain Image #" << index << ": " << swapchainImages[index] << std::endl;

        // Image View creation
        VkImageViewCreateInfo imageCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchainImages[index],
            VK_IMAGE_VIEW_TYPE_2D,
            surfaceFormats[surfaceFormatIndex].format,
            {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
        };

        assert(deviceContext->vkCreateImageView(deviceContext->device, &imageCreateInfo, nullptr, &swapchainImageViews[index]) == VK_SUCCESS);
    }

    assert(deviceContext->vkAcquireNextImageKHR(deviceContext->device, swapchain, UINT64_MAX, presentationSemaphore, VK_NULL_HANDLE, &swapchainImageIndex) == VK_SUCCESS);
    assert(swapchainImageIndex != 0xFFFFFFFF);
}

VulkanSwapchain::~VulkanSwapchain(){
    deviceContext->vkDestroySwapchainKHR(deviceContext->device, swapchain, nullptr);

    // Destroy images
    swapchainImages.clear();

    // Destroy image views
    for (auto imageView : swapchainImageViews){
        if (imageView != VK_NULL_HANDLE){
            deviceContext->vkDestroyImageView(deviceContext->device, imageView, nullptr);
        }
    }
    swapchainImageViews.clear();

    // Destroy framebuffers
    for (auto framebuffer : swapchainFramebuffers){
        if (framebuffer != VK_NULL_HANDLE){
            deviceContext->vkDestroyFramebuffer(deviceContext->device, framebuffer, nullptr);
        }
    }
    swapchainFramebuffers.clear();

    deviceContext->vkDestroySemaphore(deviceContext->device, presentationSemaphore, nullptr);
    deviceContext->vkDestroySemaphore(deviceContext->device, renderingDoneSemaphore, nullptr);
}

VkFramebuffer VulkanSwapchain::getCurrentFramebuffer(){
    if ( swapchainFramebuffers.size() == 0){
        return VK_NULL_HANDLE;
    }else{
        return swapchainFramebuffers[swapchainImageIndex];
    }
}

VkImage VulkanSwapchain::getCurrentImage(){
    if ( swapchainImages.size() == 0){
        return VK_NULL_HANDLE;
    }else{
        return swapchainImages[swapchainImageIndex];
    }
}

void VulkanSwapchain::present(VkQueue presentationQueue){
    // Get the index of the image for rendering
    uint32_t acquireTimeout         = 0x1000000; // ~16.7 ms

    std::cout << "Present - Swapchain Image Index: " << swapchainImageIndex << std::endl;

    // Present
    VkResult presentResult;
    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &renderingDoneSemaphore,
        1,
        &swapchain,
        &swapchainImageIndex,
        &presentResult
    };

    assert(deviceContext->vkQueuePresentKHR(presentationQueue, &presentInfo) == VK_SUCCESS);

    assert(deviceContext->vkAcquireNextImageKHR(deviceContext->device, swapchain, acquireTimeout, presentationSemaphore, VK_NULL_HANDLE, &swapchainImageIndex) == VK_SUCCESS);
    assert(swapchainImageIndex != 0xFFFFFFFF);
}

void VulkanSwapchain::querySwapchain(VkPhysicalDevice physicalDevice){

    // Query Swapchain image format support
    uint32_t surfaceFormatCount = 0;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr) == VK_SUCCESS);
    assert(surfaceFormatCount != 0);
    surfaceFormats = std::vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, &surfaceFormats[surfaceFormatIndex]) == VK_SUCCESS);
    std::cout << "Surface Formats: " << std::endl;
    for(auto surfaceFormat: surfaceFormats){
        std::cout << "   " << surfaceFormat.format << std::endl;
    }

    // Query Swapchain present mode support
    uint32_t presentModeCount = 0;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr) == VK_SUCCESS);
    assert(presentModeCount != 0);
    presentModes = std::vector<VkPresentModeKHR>(presentModeCount);
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, &presentModes[0]) == VK_SUCCESS);
    std::cout << "Present Modes: " << std::endl;
    for(auto presentMode: presentModes){
        std::string formatString = "";
        switch(presentMode){
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                formatString = "VK_PRESENT_MODE_IMMEDIATE_KHR";
                break;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                formatString = "VK_PRESENT_MODE_MAILBOX_KHR";
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                formatString = "VK_PRESENT_MODE_FIFO_KHR";
                break;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                formatString = "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
                break;
            default:
                formatString = "INVALID";
                break;
        }
        std::cout << "   " << formatString << std::endl;
    }

    // Query Swapchain surface capabilities
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps) == VK_SUCCESS);
    if (surfaceCaps.currentExtent.width < 1 || surfaceCaps.currentExtent.height < 1){
        extent.width    = 512;
        extent.height   = 512;
    }else{
        extent = surfaceCaps.currentExtent;
    }

    // Query surface support
    VkBool32 surfaceSupported;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &surfaceSupported) == VK_SUCCESS);
    assert(surfaceSupported == VK_TRUE);
}

void VulkanSwapchain::setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout){
    VkImageMemoryBarrier imageBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        0,
        0,
        oldLayout,
        newLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image
    };

    imageBarrier.subresourceRange.aspectMask = aspects;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.layerCount = 1;

    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageBarrier.srcAccessMask =
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageBarrier.dstAccessMask |=
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageBarrier.srcAccessMask =
                VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
    }

    deviceContext->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}

void VulkanSwapchain::setupFramebuffers(VkCommandBuffer cmdBuffer, VkRenderPass renderPass){
    uint32_t imageCount = swapchainImages.size();
    assert(imageCount != 0);

    // swapchainImageViews = std::vector<VkImageView>(imageCount);
    swapchainFramebuffers = std::vector<VkFramebuffer>(imageCount);

    for(uint32_t index = 0; index < imageCount; index++){
        // deviceContext->allocateAndBindMemory(swapchainImages[index], false);
        std::cout << "Swapchain Image #" << index << ": " << swapchainImages[index] << std::endl;

        // Image View creation
        // VkImageViewCreateInfo imageCreateInfo = {
        //     VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        //     nullptr,
        //     0,
        //     swapchainImages[index],
        //     VK_IMAGE_VIEW_TYPE_2D,
        //     surfaceFormats[surfaceFormatIndex].format,
        //     {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        //     {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
        // };

        // Set layout before creating image view
        setImageLayout(cmdBuffer, swapchainImages[index], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        // assert(deviceContext->vkCreateImageView(deviceContext->device, &imageCreateInfo, nullptr, &swapchainImageViews[index]) == VK_SUCCESS);

        // Framebuffer creation
        VkFramebufferCreateInfo framebufferCreateInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            renderPass,
            1,
            &swapchainImageViews[index],
            extent.width,
            extent.height,
            1
        };

        assert(deviceContext->vkCreateFramebuffer(deviceContext->device, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[index]) == VK_SUCCESS);
        deviceContext->allocateAndBindMemory(swapchainImages[index], false);
    }
}