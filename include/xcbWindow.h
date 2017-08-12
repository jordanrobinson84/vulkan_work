#ifndef __XCB_WINDOW__
#define __XCB_WINDOW__

#include "window.h"

class XcbWindow : public Window{
private:
public:
    XcbWindow(const uint32_t _windowWidth,
              const uint32_t _windowHeight,
              VulkanDriverInstance * __instance,
              VulkanDevice * __deviceContext,
              VkPhysicalDevice __physicalDevice,
              const std::string title);

    void createSurface();
}

#endif