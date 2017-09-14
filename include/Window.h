#ifndef __WINDOW__
#define __WINDOW__

#include "VulkanDriverInstance.h"
#include "VulkanSwapchain.h"

class Window{
private:
public:
    Window(const uint32_t _windowWidth,
           const uint32_t _windowHeight,
           VulkanDriverInstance * __instance,
           VulkanDevice * __deviceContext,
           VkPhysicalDevice __physicalDevice,
           const std::string title, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT){
        instance = __instance;
        windowWidth = _windowWidth;
        windowHeight = _windowHeight;
    };

    virtual void createSurface() = 0;

    VulkanDriverInstance*               instance;
    VkSurfaceKHR                        surface;
    VulkanSwapchain*                    swapchain;
    uint32_t                            windowHeight;
    uint32_t                            windowWidth;

    #if defined (_WIN32) || defined (_WIN64)
        HWND windowHandle;
        HINSTANCE windowInstance;
    #elif defined (__linux__)
        xcb_window_t  windowHandle;
        xcb_connection_t * windowInstance;
    #endif
};

#endif