#ifndef __VULKAN_RENDER_PASS__
#define __VULKAN_RENDER_PASS__

#include "VulkanDriverInstance.h"

class VulkanRenderPass{
public:
    VulkanRenderPass(VulkanDevice                          * __deviceContext,
                     std::vector<VkAttachmentDescription>  & attachments,
                     std::vector<VkSubpassDescription>     & subpasses,
                     std::vector<VkSubpassDependency>      & subpassDependencies);
    ~VulkanRenderPass();

    VkRenderPass   renderPass;
    VulkanDevice * deviceContext;
};

#endif