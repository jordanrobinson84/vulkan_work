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

    // Wait
    std::cin.get();
    xcb_disconnect(connection);

    return 0;
}
