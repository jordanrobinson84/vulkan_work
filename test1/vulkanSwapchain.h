#include "vulkanDriverInstance.h"

struct VulkanDevice;
class VulkanDriverInstance;

// TODO
struct VulkanSwapchain{

    VulkanSwapchain(VulkanDevice * __deviceContext, VkPhysicalDevice physicalDevice, VkSurfaceKHR swapchainSurface, std::vector<uint32_t> & supportedQueueFamilyIndices);
    ~VulkanSwapchain();
    void present(VkQueue presentationQueue);
    void querySwapchain(VkPhysicalDevice physicalDevice);
    void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspects, VkImageLayout oldLayout, VkImageLayout newLayout);
    void setupSwapchain(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);

    VulkanDevice *                  deviceContext;
    VkSurfaceKHR                    surface;
    VkSwapchainKHR                  swapchain;
    VkExtent2D                      extent;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR>   presentModes;
    std::vector<VkImage>            swapchainImages;
    std::vector<VkImageView>        swapchainImageViews;
    std::vector<VkFramebuffer>      swapchainFramebuffers;
    VkSurfaceCapabilitiesKHR        surfaceCaps;
    VkSemaphore                     presentationSemaphore;
    VkSemaphore                     renderingDoneSemaphore;
    uint32_t                        surfaceFormatIndex;
    uint32_t                        swapchainImageIndex;
};