#include "vulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(VulkanDriverInstance * __instance, VulkanDevice * __deviceContext, VkPhysicalDevice __physicalDevice, std::vector<uint32_t> & supportedQueueFamilyIndices)
: physicalDevice(__physicalDevice), queueFamilyIndices(supportedQueueFamilyIndices){
    instance            = __instance;
    deviceContext       = __deviceContext;

    assert(deviceContext != nullptr);

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

    // xcb_disconnect(connection);
}

#if defined (_WIN32) || defined (_WIN64)
    void VulkanSwapchain::createWindow(HINSTANCE hInstance,
                                       const uint32_t _windowWidth,
                                       const uint32_t _windowHeight){
        windowWidth = _windowWidth;
        windowHeight = _windowHeight;
        // Create a console
        //AllocConsole();
        //AttachConsole(GetCurrentProcessId());
        //freopen("CON", "w", stdout);
        //freopen("CON", "w", stderr);
        LPCTSTR applicationName = "Win32 Vulkan - Test 1";
        //SetConsoleTitle(applicationName);

        WNDCLASSEX win_class;

        // Initialize the window class structure:
        win_class.cbSize = sizeof(WNDCLASSEX);
        win_class.style = CS_HREDRAW | CS_VREDRAW;
        win_class.lpfnWndProc = windowProcedure;
        win_class.cbClsExtra = 0;
        win_class.cbWndExtra = 0;
        win_class.hInstance = hInstance; // hInstance
        win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        win_class.lpszMenuName = NULL;
        win_class.lpszClassName = "Vulkan";
        win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

        // Register window class:
        if (!RegisterClassEx(&win_class)) {
            // It didn't work, so try to give a useful error:
            std::cout << "Unexpected error trying to start the application!\n" << std::endl;
            //fflush(stdout);
            //exit(1);
        }

        RECT windowRect;
        int screenWidth     = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight    = GetSystemMetrics(SM_CYSCREEN);
        int left            = 100;
        int top             = 100;
        windowRect.bottom   = top + windowHeight;
        windowRect.top      = top;
        windowRect.left     = left;
        windowRect.right    = left + windowWidth;

        // Adjust the window rectangle to meet requirements
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
        windowWidth     = windowRect.right - windowRect.left;
        windowHeight    = windowRect.bottom - windowRect.top;

        // Create window
        HWND windowHandle = CreateWindowEx(0,
            "Vulkan",
            applicationName,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
            windowRect.left, windowRect.top,
            windowWidth, windowHeight,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

        if (!windowHandle) {
            std::cerr << "Couldn't create a window!" << std::endl;
            DWORD lastError = GetLastError();
            std::cerr << "Failed with error code " << lastError << std::endl;
            assert(windowHandle != nullptr);
        }

        // Get VkSurfaceKHR
        VkSurfaceKHR surface;
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.hinstance = hInstance;
        surfaceCreateInfo.hwnd = windowHandle;
        assert( instance->vkCreateSurfaceKHR(instance->instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS );

        createWindowPlatformIndependent(surface);
    }
#elif defined (__linux__)
    void VulkanSwapchain::createWindow(const uint32_t _windowWidth,
                                       const uint32_t _windowHeight){
        windowWidth     = _windowWidth;
        windowHeight    = _windowHeight;
        // Get XCB connection
        int screenNumber = 0;
        xcb_connection_t * connection = xcb_connect (nullptr, &screenNumber);

        // Get Screen 1
        const xcb_setup_t * setup               = xcb_get_setup(connection);
        xcb_screen_iterator_t  screenIterator   = xcb_setup_roots_iterator(setup);
        std::cout << "Screen Number: " << screenNumber << std::endl;
        while (screenNumber-- > 0){
            xcb_screen_next( &screenIterator);
        }

        xcb_screen_t * screen                   = screenIterator.data;

        int screenWidth  = screen->width_in_pixels;
        int screenHeight = screen->height_in_pixels;
        int left         = screenWidth / 2 - windowWidth / 2;
        int top          = screenHeight / 2 - windowHeight / 2;

        uint32_t value_list[] = {
        screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        };

        // Create window
        xcb_window_t window = xcb_generate_id(connection);
        xcb_create_window(connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            left, top,
            windowWidth, windowHeight,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            XCB_CW_BACK_PIXEL, value_list);

        // Map
        xcb_map_window(connection, window);
        xcb_flush(connection);

        // Get VkSurfaceKHR
        VkSurfaceKHR surface;
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.connection = connection;
        surfaceCreateInfo.window = window;

        assert( instance->vkCreateSurfaceKHR(instance->instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS );

        createWindowPlatformIndependent(surface);
    }
#endif

void VulkanSwapchain::createWindowPlatformIndependent(VkSurfaceKHR swapchainSurface){
    surface             = swapchainSurface;
    surfaceFormatIndex  = 0; // Default surface format
    swapchainImageIndex = 0;

    querySwapchain(physicalDevice);
    swapchainFormat = surfaceFormats[surfaceFormatIndex].format;

    // Get surface image count
    uint32_t imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0) {
        imageCount = (imageCount > surfaceCaps.maxImageCount) ? surfaceCaps.maxImageCount : imageCount;
    }

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
    swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount   = queueFamilyIndices.size();
    swapchainCreateInfo.pQueueFamilyIndices     = &queueFamilyIndices[0];
    swapchainCreateInfo.preTransform            = surfaceCaps.currentTransform;
    swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode             = presentModes[0];
    swapchainCreateInfo.clipped                 = VK_TRUE;
    swapchainCreateInfo.oldSwapchain            = VK_NULL_HANDLE;

    assert(deviceContext->vkCreateSwapchainKHR(deviceContext->device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);

    // Get Swapchain images (access-controlled)
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &imageCount, nullptr) == VK_SUCCESS);
    assert(imageCount != 0);
    swapchainImages = std::vector<VkImage>(imageCount);
    assert(deviceContext->vkGetSwapchainImagesKHR(deviceContext->device, swapchain, &imageCount, &swapchainImages[0]) == VK_SUCCESS);

    swapchainImageViews     = std::vector<VkImageView>(imageCount);
    swapchainFramebuffers   = std::vector<VkFramebuffer>(imageCount);

    for(uint32_t index = 0; index < imageCount; index++){
        std::cout << "Swapchain Image #" << index << ": " << swapchainImages[index] << std::endl;

        // Image View creation
        VkImageViewCreateInfo imageCreateInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchainImages[index],
            VK_IMAGE_VIEW_TYPE_2D,
            swapchainFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
        };

        assert(deviceContext->vkCreateImageView(deviceContext->device, &imageCreateInfo, nullptr, &swapchainImageViews[index]) == VK_SUCCESS);
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
        return swapchainImages[swapchainImageIndex];
    }
}

void VulkanSwapchain::present(VkQueue presentationQueue){
    // Get the index of the image for rendering
    uint32_t acquireTimeout         = 0x1000000; // ~16.7 ms

    // std::cout << "Present - Swapchain Image Index: " << swapchainImageIndex << std::endl;

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
    VkResult result = deviceContext->instance->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
    assert(result == VK_SUCCESS);
    if (surfaceCaps.currentExtent.width < 1 || surfaceCaps.currentExtent.height < 1 || surfaceCaps.currentExtent.width == (std::numeric_limits<uint32_t>::max)() ) { // Prevent macro expansion of conflicting windows.h max define
        extent.width    = windowWidth;
        extent.height   = windowHeight;

        extent.width    = (std::max)(surfaceCaps.minImageExtent.width, (std::min)(extent.width, surfaceCaps.maxImageExtent.width));
        extent.height   = (std::max)(surfaceCaps.minImageExtent.height, (std::min)(extent.height, surfaceCaps.maxImageExtent.height));
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

        // Set layout before creating image view
        setImageLayout(cmdBuffer, swapchainImages[index], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Framebuffer creation
        VkFramebufferCreateInfo framebufferCreateInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            renderPass,
            1,
            &(swapchainImageViews[index]),
            extent.width,
            extent.height,
            1
        };

        assert(deviceContext->vkCreateFramebuffer(deviceContext->device, &framebufferCreateInfo, nullptr, &(swapchainFramebuffers[index])) == VK_SUCCESS);
    }
}

LRESULT CALLBACK VulkanSwapchain::windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}