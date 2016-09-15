#ifndef __VULKAN_RENDER_PASS__
#define __VULKAN_RENDER_PASS__

#include "vulkanDriverInstance.h"

class VulkanRenderPass{
    VulkanRenderPass(VulkanDevice                          * __deviceContext,
                     std::vector<VkAttachmentDescription>  & attachments,
                     std::vector<VkSubpassDescription>     & subpasses,
                     std::vector<VkSubpassDependency>      & subpassDependecies = {});
    ~VulkanRenderPass();

    VkRenderPass   renderPass;
    VulkanDevice * deviceContext;
};

#endif