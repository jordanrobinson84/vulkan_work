#include "win32Window.h"

Win32Window::Win32Window(const uint32_t _windowWidth,
                         const uint32_t _windowHeight,
                         VulkanDriverInstance * __instance,
                         VulkanDevice * __deviceContext,
                         VkPhysicalDevice __physicalDevice,
                         const std::string title, VkSampleCountFlagBits sampleCount) : Window(_windowWidth, _windowHeight, __instance, __deviceContext, __physicalDevice, title, sampleCount){
    windowInstance = GetModuleHandle(nullptr);
    // Create a console
    //AllocConsole();
    //AttachConsole(GetCurrentProcessId());
    //freopen("CON", "w", stdout);
    //freopen("CON", "w", stderr);
    LPCTSTR applicationName = title.c_str();
    //SetConsoleTitle(applicationName);

    WNDCLASSEX win_class;

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = windowProcedure;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = windowInstance; // hInstance
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
    windowHandle = CreateWindowEx(0,
        "Vulkan",
        applicationName,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
        windowRect.left, windowRect.top,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        windowInstance,
        nullptr);

    if (!windowHandle) {
        std::cerr << "Couldn't create a window!" << std::endl;
        DWORD lastError = GetLastError();
        std::cerr << "Failed with error code " << lastError << std::endl;
        assert(windowHandle != nullptr);
    }

    SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR) this);

    createSurface();
    swapchain = new VulkanSwapchain(__instance, __deviceContext, __physicalDevice, surface, sampleCount);

    assert(swapchain != nullptr);
}

void Win32Window::createSurface(){
    // Get VkSurfaceKHR
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = windowInstance;
    surfaceCreateInfo.hwnd = windowHandle;
    assert( instance->vkCreateSurfaceKHR(instance->instance, &surfaceCreateInfo, nullptr, &surface) == VK_SUCCESS );
}

LRESULT CALLBACK Win32Window::handleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    uint32_t lWidth;
    uint32_t lHeight;
    switch (uMsg) 
    { 
        case WM_PAINT: 
            // Paint the window's client area. 
            std::cout << "Callback: WM_PAINT" << std::endl;
            ValidateRect(hWnd, nullptr);
            break; 
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
            std::cout << "Callback: WM_DESTROY" << std::endl;
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            exit(0);
            break; 
        case WM_SIZE:
            std::cout << "Callback: WM_SIZE" << std::endl;
            lWidth =  lParam & 0xFFFF;
            lHeight = (lParam & 0xFFFF0000) >> 16;
            std::cout << "WM_SIZE (Width, Height): " << "(" << lWidth << ", " << lHeight << ")" << std::endl;
            windowWidth = lWidth;
            windowHeight = lHeight;
            if(swapchain != nullptr && wParam == SIZE_RESTORED){
                swapchain->recreateSwapchain();
            }
            break;
        case WM_EXITSIZEMOVE:
            std::cout << "Callback: WM_EXITSIZEMOVE" << std::endl;
            PostMessage(hWnd, WM_SIZE, 0, 0);
            break;
        default: 
            // std::cout << "Callback: " << uMsg << std::endl;
            return DefWindowProc(hWnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}