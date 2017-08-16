#include "xcbWindow.h"

XcbWindow::XcbWindow(const uint32_t _windowWidth,
                     const uint32_t _windowHeight,
                     VulkanDriverInstance * __instance,
                     VulkanDevice * __deviceContext,
                     VkPhysicalDevice __physicalDevice,
                     const std::string title, VkSampleCountFlagBits) : Window(_windowWidth, _windowHeight, __instance, __deviceContext, __physicalDevice, title, sampleCount){

    // Get XCB connection
    int screenNumber = 0;
    xcb_connection_t * connection = xcb_connect (nullptr, &screenNumber);
    windowInstance = connection;

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
    windowHandle = xcb_generate_id(connection);
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

    // Set title
    xcb_change_property(connection,
        XCB_PROP_MODE_REPLACE,
        window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        sizeof(title),
        &title[0]);

    createSurface();
    swapchain = new VulkanSwapchain(__instance, __deviceContext, __physicalDevice, surface, sampleCount);

    assert(swapchain != nullptr);
}

void XcbWindow::createSurface(){
    // Get VkSurfaceKHR
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.connection = windowInstance;
    surfaceCreateInfo.window = windowHandle;

    assert( instance->vkCreateSurfaceKHR(instance->instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS );
}