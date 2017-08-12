#ifndef __WIN32_WINDOW__
#define __WIN32_WINDOW__

#include "window.h"

class Win32Window : public Window{
private:
public:
    Win32Window(const uint32_t _windowWidth,
                const uint32_t _windowHeight,
                VulkanDriverInstance * __instance,
                VulkanDevice * __deviceContext,
                VkPhysicalDevice __physicalDevice,
                const std::string title);

    void createSurface();

    LRESULT CALLBACK Win32Window::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK Win32Window::windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
        Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        if(window == nullptr){
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }else{
            return window->handleMessage(hWnd, uMsg, wParam, lParam);
        }
    };
};

#endif