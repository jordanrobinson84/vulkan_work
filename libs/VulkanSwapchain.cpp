#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanDriverInstance * __instance, VulkanDevice * __deviceContext, VkPhysicalDevice __physicalDevice, VkSurfaceKHR __surface, VkSampleCountFlagBits __sampleCount)
: physicalDevice(__physicalDevice), surface(__surface){
    instance            = __instance;
    deviceContext       = __deviceContext;
    sampleCount         = __sampleCount;
    extent.width        = -1;
    extent.height       = -1;

    assert(deviceContext != nullptr);
    pipelineState = nullptr;

    createSemaphores();
    dirtyFramebuffers = true;
    surfaceFormatIndex = 0;
    presentModeIndex = 0;

    // Get usable queues for presentation
    uint32_t queueFamilyIndex = 0;
    // Enumerate Physical Device Queue Family Properties
    uint32_t deviceQueueFamilyPropertyCount = 0;
    instance->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertyCount, nullptr);
    assert( deviceQueueFamilyPropertyCount != 0);
    while(queueFamilyIndex < deviceQueueFamilyPropertyCount){
        VkBool32 presentationSupported = VK_FALSE;

        VkResult result = instance->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentationSupported);
        if(result == VK_SUCCESS && presentationSupported == VK_TRUE){
            queueFamilyIndices.push_back(queueFamilyIndex);
        }
        queueFamilyIndex++;
    }
    assert(queueFamilyIndices.size() > 0);

    // Initialize
    initializeSwapchain(surface, VK_NULL_HANDLE);
}

VulkanSwapchain::~VulkanSwapchain(){
    cleanupSwapchain();
    deviceContext->vkDestroySwapchainKHR(deviceContext->device, swapchain, nullptr);
}

void VulkanSwapchain::cleanupSwapchain(){
    // Destroy images
    swapchainImages.clear();

    for (auto depthImage : swapchainDepthImages){
        if (depthImage != nullptr){
            delete depthImage;
        }
    }
    swapchainDepthImages.clear();

    for (auto multisampleImage : swapchainMultisampleImages){
        if (multisampleImage != nullptr){
            delete multisampleImage;
        }
    }
    swapchainMultisampleImages.clear();

    // Destroy images
    for (auto image : swapchainImages){
        if (image != nullptr){
            delete image;
        }
    }
    swapchainImages.clear();

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

void VulkanSwapchain::createRenderpass(){
    assert(pipelineState != nullptr);

    // Create render pass
    VkAttachmentDescription defaultAttachment;
    defaultAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    defaultAttachment.format = surfaceFormats[surfaceFormatIndex].format;
    defaultAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    defaultAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    defaultAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    defaultAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    defaultAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    defaultAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    defaultAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttachment;
    depthAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    depthAttachment.format = swapchainDepthFormat;
    depthAttachment.samples = sampleCount;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentDescription> attachments = {defaultAttachment, depthAttachment};

    if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
        // Multisample color attachment
        VkAttachmentDescription multisampleAttachment;
        multisampleAttachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
        multisampleAttachment.format = surfaceFormats[surfaceFormatIndex].format;
        multisampleAttachment.samples = sampleCount;
        multisampleAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        multisampleAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        multisampleAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        multisampleAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        multisampleAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        multisampleAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(multisampleAttachment);
    }

    VkAttachmentReference colorAttachmentReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthAttachmentReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    std::vector<VkAttachmentReference> attachmentReferences = {colorAttachmentReference, depthAttachmentReference};
    if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
        VkAttachmentReference multisampleColorAttachmentReference = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        attachmentReferences.push_back(multisampleColorAttachmentReference);
    }

    VkSubpassDescription subpassDescription;
    subpassDescription.flags                    = 0; // Flags
    subpassDescription.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS; // pipelineBindPoint
    subpassDescription.inputAttachmentCount     = 0; // inputAttachmentCount
    subpassDescription.pInputAttachments        = nullptr; // pInputAttachments
    subpassDescription.colorAttachmentCount     = 1; // colorAttachmentCount
    subpassDescription.pColorAttachments        = &attachmentReferences[0]; // pColorAttachments
    subpassDescription.pResolveAttachments      = nullptr; // pResolveAttachments
    subpassDescription.pDepthStencilAttachment  = &attachmentReferences[1]; // pDepthStencilAttachment
    if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
        // When multisampling, the multisample attachment is used for rendering and is resolved in the non-multisampled attachment later
        subpassDescription.pColorAttachments        = &attachmentReferences[2]; // pColorAttachments
        subpassDescription.pResolveAttachments      = &attachmentReferences[0];
    }
    subpassDescription.preserveAttachmentCount  = 0; // preserveAttachmentCount
    subpassDescription.pPreserveAttachments     = nullptr; // pPreserveAttachments

    std::vector<VkSubpassDescription> subpasses     = {subpassDescription};
    std::vector<VkSubpassDependency> dependencies   = {};

    uint32_t attachmentCount        = attachments.size();
    uint32_t subpassCount           = subpasses.size();
    uint32_t subpassDependencyCount = dependencies.size();

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext              = nullptr;
    renderPassCreateInfo.flags              = 0;
    renderPassCreateInfo.attachmentCount    = attachmentCount;
    renderPassCreateInfo.pAttachments       = &attachments[0];
    renderPassCreateInfo.subpassCount       = subpassCount;
    renderPassCreateInfo.pSubpasses         = &subpasses[0];
    renderPassCreateInfo.dependencyCount    = 0;
    renderPassCreateInfo.pDependencies      = nullptr;

    // Add subpasses if they exist
    if (subpassDependencyCount > 0){
        renderPassCreateInfo.dependencyCount = subpassDependencyCount;
        renderPassCreateInfo.pDependencies   = &dependencies[0];
    }

    assert(deviceContext->vkCreateRenderPass(deviceContext->device, &renderPassCreateInfo, nullptr, &renderPass) == VK_SUCCESS);

    // Blend states
    attachmentBlendState.blendEnable            = VK_FALSE;
    attachmentBlendState.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE;
    attachmentBlendState.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.colorBlendOp           = VK_BLEND_OP_ADD;
    attachmentBlendState.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE;
    attachmentBlendState.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.alphaBlendOp           = VK_BLEND_OP_ADD;
    attachmentBlendState.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    blendState.sType                = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.pNext                = nullptr;
    blendState.flags                = 0; //MBZ
    blendState.logicOpEnable        = VK_FALSE;
    blendState.logicOp              = VK_LOGIC_OP_NO_OP;
    blendState.attachmentCount      = subpasses[0].colorAttachmentCount;
    blendState.pAttachments         = &attachmentBlendState;
    blendState.blendConstants[0]    = 1.0;
    blendState.blendConstants[1]    = 1.0;
    blendState.blendConstants[2]    = 1.0;
    blendState.blendConstants[3]    = 1.0;
    pipelineState->pipelineInfo.pColorBlendState = &blendState;

    pipelineState->pipelineInfo.renderPass = renderPass;

    pipelineState->setMultisampleState(sampleCount);
}

void VulkanSwapchain::createSemaphores(){
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
}

void VulkanSwapchain::initializeSwapchain(VkSurfaceKHR swapchainSurface, VkSwapchainKHR oldSwapchain){
    surface             = swapchainSurface;
    surfaceFormatIndex  = 0; // Default surface format
    swapchainImageIndex = 0;

    querySwapchain();
    swapchainFormat = surfaceFormats[surfaceFormatIndex].format;
    std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    swapchainDepthFormat = deviceContext->getSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    assert(swapchainDepthFormat != VK_FORMAT_UNDEFINED);
    std::cout << "Depth Format: " << swapchainDepthFormat << std::endl;

    // Enable MSAA
    setupMultisampling(sampleCount);

    // Get surface image count
    imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0) {
        imageCount = (imageCount > surfaceCaps.maxImageCount) ? surfaceCaps.maxImageCount : imageCount;
    }

    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if(surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR == 0){
        preTransform = surfaceCaps.currentTransform;
    }

    VkSharingMode imageSharingMode = queueFamilyIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    std::cout << "Image Sharing Mode: " << (queueFamilyIndices.size() > 1 ? "VK_SHARING_MODE_CONCURRENT" : "VK_SHARING_MODE_EXCLUSIVE") << std::endl;

    // Swapchain creation info
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext                   = nullptr;
    swapchainCreateInfo.flags                   = 0;
    swapchainCreateInfo.surface                 = surface;
    swapchainCreateInfo.minImageCount           = imageCount;
    swapchainCreateInfo.imageFormat             = swapchainFormat;
    swapchainCreateInfo.imageColorSpace         = surfaceFormats[surfaceFormatIndex].colorSpace;
    swapchainCreateInfo.imageExtent             = extent;
    swapchainCreateInfo.imageArrayLayers        = 1;
    swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapchainCreateInfo.imageSharingMode        = imageSharingMode;
    swapchainCreateInfo.queueFamilyIndexCount   = queueFamilyIndices.size();
    swapchainCreateInfo.pQueueFamilyIndices     = (imageSharingMode == VK_SHARING_MODE_CONCURRENT) ? &queueFamilyIndices.at(0) : nullptr;
    swapchainCreateInfo.preTransform            = preTransform;
    swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode             = presentModes[presentModeIndex];
    swapchainCreateInfo.clipped                 = VK_TRUE;
    swapchainCreateInfo.oldSwapchain            = oldSwapchain;

    std::cout << "Pre-Create Swapchain - Current Swapchain: " << swapchain << "/Old Swapchain: " << oldSwapchain << std::endl;
    assert(deviceContext->vkCreateSwapchainKHR(deviceContext->device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);
    std::cout << "Post-Create Swapchain - Current Swapchain: " << swapchain << "/Old Swapchain: " << oldSwapchain << std::endl;
    if(oldSwapchain != VK_NULL_HANDLE){
        deviceContext->vkDestroySwapchainKHR(deviceContext->device, oldSwapchain, nullptr);
    }

    // Get Swapchain images (access-controlled)
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &imageCount, nullptr) == VK_SUCCESS);
    assert(imageCount != 0);
    std::vector<VkImage> swapchainPrimitiveImages(imageCount);
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &imageCount, &swapchainPrimitiveImages[0]) == VK_SUCCESS);

    swapchainImages             = std::vector<VulkanImage*>(imageCount);
    swapchainDepthImages        = std::vector<VulkanImage*>(imageCount);
    swapchainFramebuffers       = std::vector<VkFramebuffer>(imageCount);

    if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
        swapchainMultisampleImages   = std::vector<VulkanImage*>(imageCount);
    }

    for(uint32_t index = 0; index < imageCount; index++){
        // Create image wrapper for WSI image
        swapchainImages[index] = new VulkanImage(deviceContext,
                                                swapchainPrimitiveImages[index],
                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                VK_IMAGE_TYPE_2D,
                                                swapchainFormat,
                                                {extent.width, extent.height, 1},
                                                0,
                                                VK_SAMPLE_COUNT_1_BIT,
                                                VK_IMAGE_TILING_OPTIMAL,
                                                1,
                                                1,
                                                imageSharingMode,
                                                queueFamilyIndices.size(),
                                                (imageSharingMode == VK_SHARING_MODE_CONCURRENT) ? &queueFamilyIndices.at(0) : nullptr,
                                                VK_IMAGE_LAYOUT_UNDEFINED);
        swapchainImages[index]->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
        std::cout << "Swapchain Image #" << index << ": " << swapchainImages[index] << std::endl;

        // Depth buffer images must be created
        swapchainDepthImages[index] = new VulkanImage(deviceContext,
                                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                       VK_IMAGE_TYPE_2D,
                                                       swapchainDepthFormat,
                                                       {extent.width, extent.height, 1},
                                                       0,
                                                       sampleCount,
                                                       VK_IMAGE_TILING_OPTIMAL,
                                                       1,
                                                       1,
                                                       imageSharingMode,
                                                       queueFamilyIndices.size(),
                                                       (imageSharingMode == VK_SHARING_MODE_CONCURRENT) ? &queueFamilyIndices.at(0) : nullptr,
                                                       VK_IMAGE_LAYOUT_UNDEFINED);
        swapchainDepthImages[index]->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0);
        std::cout << "Swapchain Depth Image #" << index << ": " << swapchainDepthImages[index]->imageHandle << std::endl;

        // Create multisampled color and depth buffers
        if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
            // Create multisampled color buffer
            swapchainMultisampleImages[index] = new VulkanImage(deviceContext,
                                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                               VK_IMAGE_TYPE_2D,
                                                               swapchainFormat,
                                                               {extent.width, extent.height, 1},
                                                               0,
                                                               sampleCount,
                                                               VK_IMAGE_TILING_OPTIMAL,
                                                               1,
                                                               1,
                                                               imageSharingMode,
                                                               queueFamilyIndices.size(),
                                                               (imageSharingMode == VK_SHARING_MODE_CONCURRENT) ? &queueFamilyIndices.at(0) : nullptr,
                                                               VK_IMAGE_LAYOUT_UNDEFINED);
            swapchainMultisampleImages[index]->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
            std::cout << "Multisampled Swapchain Image #" << index << ": " << swapchainMultisampleImages[index]->imageHandle << std::endl;
        }
    }

    assert(deviceContext->vkAcquireNextImageKHR(deviceContext->device, swapchain, UINT64_MAX, presentationSemaphore, VK_NULL_HANDLE, &swapchainImageIndex) == VK_SUCCESS);
    assert(swapchainImageIndex != 0xFFFFFFFF);

    std::cout << "Window Creation Complete!" << std::endl;
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
        return swapchainImages[swapchainImageIndex]->imageHandle;
    }
}

void VulkanSwapchain::present(VkQueue presentationQueue){
    // Get the index of the image for rendering
    uint32_t acquireTimeout         = 0x1000000; // ~16.7 ms

    // std::cout << "Present - Swapchain Image Index: " << swapchainImageIndex << std::endl;

    // Present
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderingDoneSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &swapchainImageIndex;
    presentInfo.pResults = nullptr;

    VkResult presentResult = deviceContext->vkQueuePresentKHR(presentationQueue, &presentInfo);
    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR){
        std::cout << "vkQueuePresentKHR status:VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
        recreateSwapchain();
    }else if(presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to present swapchain!");
    }

    VkResult acquireResult = deviceContext->vkAcquireNextImageKHR(deviceContext->device, swapchain, acquireTimeout, presentationSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);
    if(acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR){
        std::cout << "vkAcquireNextImageKHR status:" << (acquireResult == VK_ERROR_OUT_OF_DATE_KHR ? "VK_ERROR_OUT_OF_DATE_KHR" : "VK_SUBOPTIMAL_KHR") << std::endl;
        recreateSwapchain();
    }else if(acquireResult != VK_SUCCESS && acquireResult != VK_TIMEOUT && acquireResult != VK_NOT_READY){
        std::string errorString = "";
        switch(acquireResult){
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                errorString = "VK_ERROR_OUT_OF_HOST_MEMORY";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                errorString = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
                break;
            case VK_ERROR_DEVICE_LOST:
                errorString = "VK_ERROR_DEVICE_LOST";
                break;
            case VK_ERROR_OUT_OF_DATE_KHR: // This SHOULD never be hit, but just in case
                errorString = "VK_ERROR_OUT_OF_DATE_KHR";
                break;
            case VK_ERROR_SURFACE_LOST_KHR:
                errorString = "VK_ERROR_SURFACE_LOST_KHR";
                break;   
            default:
                errorString += acquireResult;
            }
        throw std::runtime_error("Failed to acquire swapchain image!" + errorString);
    }
    assert(swapchainImageIndex != 0xFFFFFFFF);
}

void VulkanSwapchain::querySwapchain(){

    // Query Swapchain image format support
    uint32_t surfaceFormatCount = 0;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr) == VK_SUCCESS);
    assert(surfaceFormatCount != 0);
    surfaceFormats = std::vector<VkSurfaceFormatKHR>(surfaceFormatCount);
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, &surfaceFormats[surfaceFormatIndex]) == VK_SUCCESS);
    std::cout << "Surface Formats: " << std::endl;
    int formatIndex = 0;
    if(surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED){
        surfaceFormatIndex = 0;
    }
    for(auto surfaceFormat: surfaceFormats){
        std::cout << "   " << surfaceFormat.format << std::endl;
        if(surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ){
            surfaceFormatIndex = formatIndex;
        }// TODO: Rank formats

        formatIndex++;
    }

    std::cout << "Chosen Surface Format: " << surfaceFormats[surfaceFormatIndex].format << std::endl;

    // Query Swapchain present mode support
    uint32_t presentModeCount = 0;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr) == VK_SUCCESS);
    assert(presentModeCount != 0);
    presentModes = std::vector<VkPresentModeKHR>(presentModeCount);
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, &presentModes[0]) == VK_SUCCESS);
    std::cout << "Present Modes: " << std::endl;
    int modeIndex = 0;
    std::vector<std::string> presentModeStrings;
    for(auto presentMode: presentModes){
        std::string formatString = "";
        switch(presentMode){
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                formatString = "VK_PRESENT_MODE_IMMEDIATE_KHR";
                if(presentModes[presentModeIndex] != VK_PRESENT_MODE_MAILBOX_KHR && presentModes[presentModeIndex] != VK_PRESENT_MODE_FIFO_KHR){
                    presentModeIndex = modeIndex;
                }
                break;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                formatString = "VK_PRESENT_MODE_MAILBOX_KHR";
                if(presentModes[presentModeIndex] != VK_PRESENT_MODE_MAILBOX_KHR){
                    presentModeIndex = modeIndex;
                }
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                formatString = "VK_PRESENT_MODE_FIFO_KHR";
                presentModeIndex = modeIndex;
                break;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                formatString = "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
                break;
            default:
                formatString = "INVALID";
                break;
        }
        std::cout << "   " << formatString << std::endl;
        presentModeStrings.push_back(formatString);
        modeIndex++;
    }

    std::cout << "Chosen Presentation Mode: " << presentModeStrings[presentModeIndex] << std::endl;

    // Query Swapchain surface capabilities
    VkResult result = deviceContext->instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
    assert(result == VK_SUCCESS);
    if (surfaceCaps.currentExtent.width == (std::numeric_limits<uint32_t>::max)() ) { // Prevent macro expansion of conflicting windows.h max define
        std::cout << "Calculating Swapchain Extents" << std::endl;
        extent.width    = swapchainWidth;
        extent.height   = swapchainHeight;

        extent.width    = (std::max)(surfaceCaps.minImageExtent.width, (std::min)(extent.width, surfaceCaps.maxImageExtent.width));
        extent.height   = (std::max)(surfaceCaps.minImageExtent.height, (std::min)(extent.height, surfaceCaps.maxImageExtent.height));
    }else{
        extent = surfaceCaps.currentExtent;
    }

    std::cout << "Swapchain Extents: [" << extent.width << ", " << extent.height << "]" << std::endl;

    // Query surface support
    VkBool32 surfaceSupported = false;
    assert(deviceContext->instance->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &surfaceSupported) == VK_SUCCESS);
    assert(surfaceSupported == VK_TRUE);
}

void VulkanSwapchain::recreateSwapchain(){
    std::cout << "Recreating swapchain!" << std::endl;
    dirtyFramebuffers = true;
    assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
    VkSwapchainKHR oldSwapchain = swapchain;
    cleanupSwapchain();
    createSemaphores();
    initializeSwapchain(surface, oldSwapchain);
    createRenderpass();
    VkRect2D scissorRect = { { 0, 0 }, extent };
    pipelineState->setViewportState(extent, scissorRect);
}

void VulkanSwapchain::setImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout){
    if ( swapchainImages.size() != 0){
        swapchainImages[swapchainImageIndex]->setImageLayout(commandBuffer, oldLayout, newLayout);
    }
}

void VulkanSwapchain::setPipelineState(VulkanPipelineState *vps){
    pipelineState = vps;
}

void VulkanSwapchain::setupFramebuffers(VkCommandBuffer cmdBuffer){
    imageCount = swapchainImages.size();
    assert(imageCount != 0);

    swapchainFramebuffers = std::vector<VkFramebuffer>(imageCount);

    for(uint32_t index = 0; index < imageCount; index++){
        std::cout << "Swapchain Image #" << index << ": " << swapchainImages[index]->imageHandle << std::endl;

        // Set layout before creating image view
        swapchainImages[index]->setImageLayout(cmdBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Framebuffer creation
        std::vector<VkImageView> attachments;
        attachments.push_back(swapchainImages[index]->imageViewHandle);
        attachments.push_back(swapchainDepthImages[index]->imageViewHandle);
        if(sampleCount != VK_SAMPLE_COUNT_1_BIT){
            attachments.push_back(swapchainMultisampleImages[index]->imageViewHandle);
        }

        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext             = nullptr;
        framebufferCreateInfo.flags             = 0;
        framebufferCreateInfo.renderPass        = renderPass;
        framebufferCreateInfo.attachmentCount   = attachments.size();
        framebufferCreateInfo.pAttachments      = &attachments[0];
        framebufferCreateInfo.width             = extent.width;
        framebufferCreateInfo.height            = extent.height;
        framebufferCreateInfo.layers            = 1;

        assert(deviceContext->vkCreateFramebuffer(deviceContext->device, &framebufferCreateInfo, nullptr, &(swapchainFramebuffers[index])) == VK_SUCCESS);
    }

    dirtyFramebuffers = false;
}

void VulkanSwapchain::setupMultisampling(VkSampleCountFlagBits __sampleCount){
    sampleCount = __sampleCount;

    // Get Device Format Properties for Framebuffers
    uint32_t deviceNumber = deviceContext->deviceNumber;
    VkImageFormatProperties colorFormatProperties;
    VkImageFormatProperties depthFormatProperties;
    deviceContext->instance->vkGetPhysicalDeviceImageFormatProperties(deviceContext->getPhysicalDevice(),
                                                                      swapchainFormat,
                                                                      VK_IMAGE_TYPE_2D, 
                                                                      VK_IMAGE_TILING_OPTIMAL,
                                                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                      0,
                                                                      &colorFormatProperties);

    deviceContext->instance->vkGetPhysicalDeviceImageFormatProperties(deviceContext->getPhysicalDevice(),
                                                                      swapchainDepthFormat,
                                                                      VK_IMAGE_TYPE_2D, 
                                                                      VK_IMAGE_TILING_OPTIMAL,
                                                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                      0,
                                                                      &depthFormatProperties);

    VkSampleCountFlagBits sampleCountFlag;
    VkSampleCountFlags sampleCountLimits = colorFormatProperties.sampleCounts & depthFormatProperties.sampleCounts;

    // Sanitize input to only use valid values
    std::cout << "Requested sample count {" << std::hex << sampleCount << "}." << std::endl;
    switch(sampleCount){
        case 1:
            sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
            break;
        case 2:
            sampleCountFlag = VK_SAMPLE_COUNT_2_BIT;
            break;
        case 4:
            sampleCountFlag = VK_SAMPLE_COUNT_4_BIT;
            break;
        case 8:
            sampleCountFlag = VK_SAMPLE_COUNT_8_BIT;
            break;
        case 16:
            sampleCountFlag = VK_SAMPLE_COUNT_16_BIT;
            break;
        case 32:
            sampleCountFlag = VK_SAMPLE_COUNT_32_BIT;
            break;
        case 64:
            sampleCountFlag = VK_SAMPLE_COUNT_64_BIT;
            break;
        default:
            sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
    }

    std::cout << "Sanitized sample count {" << std::hex << sampleCountFlag << "}." << std::endl;
    std::cout << "Sample count limits {" << std::hex << sampleCountLimits << "}." << std::endl;
    // If selected sample count isn't supported, find the closest supported count
    if((sampleCountFlag & sampleCountLimits) == 0){
        std::cout << "Requested sample count is not supported." << std::endl;

        while((sampleCountFlag & sampleCountLimits) == 0){
            sampleCountFlag = (VkSampleCountFlagBits)((uint32_t)sampleCountFlag >> 1);
            std::cout << "Candidate sample count {" << sampleCountFlag << "}" << std::endl;
        }
        std::cout << "Falling back to closest supported count {" << sampleCountFlag << "}" << std::endl;
    }

    sampleCount = sampleCountFlag;
}