#ifndef __VULKAN_SWAPCHAIN__
#define __VULKAN_SWAPCHAIN__

#include "VulkanDriverInstance.h"
#include "VulkanPipelineState.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"

struct VulkanDevice;
class VulkanDriverInstance;

// TODO
struct VulkanSwapchain{
private:
public:
    VulkanSwapchain(VulkanDriverInstance * __instance, VulkanDevice * __deviceContext, VkPhysicalDevice __physicalDevice, VkSurfaceKHR __surface, VkSampleCountFlagBits __sampleCount = VK_SAMPLE_COUNT_1_BIT);
    ~VulkanSwapchain();
    void cleanupSwapchain();
    void createRenderpass();
    void createSemaphores();
    VkFramebuffer getCurrentFramebuffer();
    VkImage       getCurrentImage();
    void initializeSwapchain(VkSurfaceKHR swapchainSurface, VkSwapchainKHR oldSwapchain);
    void present(VkQueue presentationQueue);
    void querySwapchain();
    void recreateSwapchain();
    void setImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
    void setPipelineState(VulkanPipelineState *vps);
    void setupFramebuffers(VkCommandBuffer cmdBuffer);
    void setupMultisampling(VkSampleCountFlagBits __sampleCount);

    VulkanDriverInstance *              instance;
    VulkanDevice *                      deviceContext;
    VkPipelineColorBlendAttachmentState attachmentBlendState;
    VkPipelineColorBlendStateCreateInfo blendState;
    bool                                dirtyFramebuffers;
    VkPhysicalDevice                    physicalDevice;
    VkSurfaceKHR                        surface;
    VkSwapchainKHR                      swapchain;
    VkExtent2D                          extent;
    std::vector<VkSurfaceFormatKHR>     surfaceFormats;
    std::vector<VkPresentModeKHR>       presentModes;
    uint32_t                            imageCount;
    VkSampleCountFlagBits               sampleCount;
    std::vector<VulkanImage*>           swapchainDepthImages;
    std::vector<VulkanImage*>           swapchainImages;

    // Multisampled resources
    std::vector<VulkanImage*>           swapchainMultisampleImages;

    std::vector<VkFramebuffer>          swapchainFramebuffers;
    std::vector<uint32_t>               queueFamilyIndices;
    VkSurfaceCapabilitiesKHR            surfaceCaps;
    VulkanPipelineState *               pipelineState;
    uint32_t                            presentModeIndex;
    VkSemaphore                         presentationSemaphore;
    VkSemaphore                         renderingDoneSemaphore;
    VkRenderPass                        renderPass;
    uint32_t                            surfaceFormatIndex;
    VkFormat                            swapchainDepthFormat;
    VkFormat                            swapchainFormat;
    uint32_t                            swapchainImageIndex;
    uint32_t                            swapchainHeight;
    uint32_t                            swapchainWidth;
};

#endif