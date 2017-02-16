#include "vulkanDriverInstance.h"

struct VulkanDevice;
class VulkanDriverInstance;

// TODO
struct VulkanSwapchain{
private:
    void createWindowPlatformIndependent(VkSurfaceKHR swapchainSurface);
public:
    VulkanSwapchain(VulkanDriverInstance * __instance, VulkanDevice * __deviceContext, VkPhysicalDevice __physicalDevice, std::vector<uint32_t> & supportedQueueFamilyIndices);
    ~VulkanSwapchain();
#if defined (_WIN32) || defined (_WIN64)
    void createWindow(HINSTANCE hInstance,
                      const uint32_t _windowWidth,
                      const uint32_t _windowHeight);
#elif defined (__linux__)
    void createWindow(const uint32_t _windowWidth,
                      const uint32_t _windowHeight);
#endif
    VkFramebuffer getCurrentFramebuffer();
    VkImage       getCurrentImage();
    void present(VkQueue presentationQueue);
    void querySwapchain(VkPhysicalDevice physicalDevice);
    void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout);
    void setupFramebuffers(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);
    // void setupSwapchain(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);
#if defined (_WIN32) || defined (_WIN64)
    static LRESULT CALLBACK VulkanSwapchain::windowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

    VulkanDriverInstance *          instance;
    VulkanDevice *                  deviceContext;
    VkPhysicalDevice                physicalDevice;
    VkSurfaceKHR                    surface;
    VkSwapchainKHR                  swapchain;
    VkExtent2D                      extent;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR>   presentModes;
    std::vector<VkImage>            swapchainImages;
    std::vector<VkImageView>        swapchainImageViews;
    std::vector<VkFramebuffer>      swapchainFramebuffers;
    std::vector<uint32_t> &         queueFamilyIndices;
    VkSurfaceCapabilitiesKHR        surfaceCaps;
    VkSemaphore                     presentationSemaphore;
    VkSemaphore                     renderingDoneSemaphore;
    uint32_t                        surfaceFormatIndex;
    VkFormat                        swapchainFormat;
    uint32_t                        swapchainImageIndex;
    uint32_t                        windowHeight;
    uint32_t                        windowWidth;
};