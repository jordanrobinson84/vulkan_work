#include <iostream>
#include <cassert>
#include "vulkanDriverInstance.h"
#include "vulkanBuffer.h"

int main(){
    VulkanDriverInstance instance("Linux");

    if(instance.loader == nullptr){
        std::cout << "Vulkan library not found!" << std::endl;
        return false;
    }
    std::cout << "Vulkan library found!" << std::endl;
    if (instance.vkGetInstanceProcAddr != nullptr){
        std::cout << "vkGetInstanceProcAddr function found!" << std::endl;
    }
    if (instance.vkCreateInstance != nullptr){
        std::cout << "vkCreateInstance function found!" << std::endl;
    }

    // Set up instance variables and functions
    VulkanDevice * deviceContext = nullptr;
    deviceContext = instance.getDevice(0);
    if (deviceContext == nullptr){
        std::cout << "Could not create a Vulkan Device!" << std:: endl;
        return false;
    }

    // Set buffer data
    float vertexBufferData[9];
    vertexBufferData[0] = 0.2; // x[0]
    vertexBufferData[1] = 0.2; // y[0]
    vertexBufferData[2] = 0.0; // z[0]
    vertexBufferData[3] = 0.2; // x[1]
    vertexBufferData[4] = 0.8; // y[1]
    vertexBufferData[5] = 0.0; // z[1]
    vertexBufferData[6] = 0.8; // x[2]
    vertexBufferData[7] = 0.5; // y[2]
    vertexBufferData[8] = 0.0; // z[2]

    VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferData, sizeof(float) * 9, false);

    // Get XCB connection
    xcb_connection_t * connection = xcb_connect (nullptr, nullptr);

    // Get Screen 1
    const xcb_setup_t * setup               = xcb_get_setup(connection);
    xcb_screen_iterator_t  screenIterator   = xcb_setup_roots_iterator(setup);
    xcb_screen_t * screen                   = screenIterator.data;

    // Create window
    xcb_window_t window = xcb_generate_id(connection);
    xcb_create_window(connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0, 0,
        512, 512,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        0, nullptr);

    // Get VkSurfaceKHR
    VkSurfaceKHR surface;
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.connection = connection;
    surfaceCreateInfo.window = window;

    assert(vkCreateSurfaceKHR(instance.instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS);

    // Map
    xcb_map_window(connection, window);
    xcb_flush(connection);

    // Create a swapchain
    VkSwapchainKHR swapchain;

    // Query Swapchain image format support
    uint32_t surfaceFormatCount = 0;
    assert(vkGetPhysicalDeviceSurfaceFormatsKHR(instance.physicalDevices[0], surface, &surfaceFormatCount, nullptr) == VK_SUCCESS);
    assert(surfaceFormatCount != 0);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    assert(vkGetPhysicalDeviceSurfaceFormatsKHR(instance.physicalDevices[0], surface, &surfaceFormatCount, &surfaceFormats[0]) == VK_SUCCESS);
    std::cout << "Surface Formats: " << std::endl;
    for(auto surfaceFormat: surfaceFormats){
        std::cout << "   " << surfaceFormat.format << std::endl;
    }

    // Query Swapchain present mode support
    uint32_t presentModeCount = 0;
    assert(vkGetPhysicalDeviceSurfacePresentModesKHR(instance.physicalDevices[0], surface, &presentModeCount, nullptr) == VK_SUCCESS);
    assert(presentModeCount != 0);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    assert(vkGetPhysicalDeviceSurfacePresentModesKHR(instance.physicalDevices[0], surface, &presentModeCount, &presentModes[0]) == VK_SUCCESS);
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
    VkSurfaceCapabilitiesKHR surfaceCaps;
    assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(instance.physicalDevices[0], surface, &surfaceCaps) == VK_SUCCESS);
    VkExtent2D extent;
    if (surfaceCaps.currentExtent.width < 1 || surfaceCaps.currentExtent.height < 1){
        extent.width    = 512;
        extent.height   = 512;
    }else{
        extent = surfaceCaps.currentExtent;
    }

    // Query surface support
    VkBool32 surfaceSupported;
    assert(vkGetPhysicalDeviceSurfaceSupportKHR(instance.physicalDevices[0], 0, surface, &surfaceSupported) == VK_SUCCESS);
    assert(surfaceSupported == VK_TRUE);

    // Create presentation semaphore
    VkSemaphoreCreateInfo presentatonSemaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    }
    VkSemaphore presentationSemaphore;
    assert(vkCreateSemaphore(deviceContext->device, &presentatonSemaphoreCreateInfo, nullptr, &presentationSemaphore) == VK_SUCCESS);

    // Create rendering done semaphore
    VkSemaphoreCreateInfo renderingDoneSemaphoreCreateInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    }
    VkSemaphore renderingDoneSemaphore;
    assert(vkCreateSemaphore(deviceContext->device, &renderingDoneSemaphoreCreateInfo, nullptr, &renderingDoneSemaphore) == VK_SUCCESS);

    // Swapchain creation info
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext                   = nullptr;
    swapchainCreateInfo.flags                   = surfaceFormats[0].colorSpace;
    swapchainCreateInfo.surface                 = surface;
    swapchainCreateInfo.minImageCount           = surfaceCaps.minImageCount;
    swapchainCreateInfo.imageFormat             = surfaceFormats[0].format;
    swapchainCreateInfo.imageColorSpace         = surfaceFormats[0].colorSpace;
    swapchainCreateInfo.imageExtent             = extent;
    swapchainCreateInfo.imageArrayLayers        = 1;
    swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount   = 0; // Not shared
    swapchainCreateInfo.pQueueFamilyIndices     = nullptr; // Not shared
    swapchainCreateInfo.preTransform            = surfaceCaps.currentTransform;
    swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode             = presentModes[0];
    swapchainCreateInfo.clipped                 = VK_TRUE;
    swapchainCreateInfo.oldSwapchain            = nullptr;

    assert(vkCreateSwapchainKHR(deviceContext->device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);

    // Get Swapchain images (access-controlled)
    uint32_t swapchainImageCount = 0;
    assert(vkGetSwapchainImages(deviceContext->device, swapchain, &swapchainImageCount, nullptr) == VK_SUCCESS);
    assert(swapchainImageCount != 0);
    std::vector<VkImage> swapchainImages(swapchainImageCount);
    assert(vkGetSwapchainImages(deviceContext->device, swapchain, &swapchainImageCount, &swapchainImages[0]) == VK_SUCCESS);

    // Get presentation queue
    int32_t presentationQueueFamily = deviceContext->getUsableDeviceQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    assert(presentationQueueFamily != -1);
    VkQueue presentationQueue;
    assert(deviceContext->vkGetDeviceQueue(deviceContext->device, presentationQueueFamily, 0, &presentationQueue) == VK_SUCCESS);

    // Do rendering
    // 
    // 

    // Get the index of the image for rendering
    uint32_t swapchainImageIndex    = 0xFFFFFFFF;
    uint32_t acquireTimeout         = 0x1000000; // ~16.7 ms
    assert(vkAcquireNextImageKHR(deviceContext->device, swapchain, acquireTimeout, presentationSemaphore, VK_NULL_HANDLE, &swapchainImageIndex) == VK_SUCCESS);
    assert(swapchainImageIndex != 0xFFFFFFFF);

    // Present
    VkResult presentResult;
    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        &renderingDoneSemaphore,
        1,
        &swapchain,
        &swapchainImageIndex,
        &presentResult
    }

    assert(vkQueuePresentKHR(presentationQueue, &presentInfo) == VK_SUCCESS);

    // Wait
    std::cin.get();
    vkDestroySwapchainKHR(deviceContext->device, swapchain, nullptr);
    xcb_disconnect(connection);

    return 0;
}
