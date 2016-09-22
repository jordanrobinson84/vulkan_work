#include "vulkanRenderPass.h"

VulkanRenderPass::VulkanRenderPass(VulkanDevice                          * __deviceContext,
                                   std::vector<VkAttachmentDescription>  & attachments,
                                   std::vector<VkSubpassDescription>     & subpasses,
                                   std::vector<VkSubpassDependency>      & subpassDependencies){
    deviceContext = __deviceContext;
    assert(deviceContext != nullptr);
    assert(attachments.size() > 0);
    assert(subpasses.size() > 0);

    uint32_t attachmentCount        = attachments.size();
    uint32_t subpassCount           = subpasses.size();
    uint32_t subpassDependencyCount = subpassDependencies.size();

    VkRenderPassCreateInfo renderPassCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        attachmentCount,
        &attachments[0],
        subpassCount,
        &subpasses[0],
        0,
        nullptr
    };

    // Add subpasses if they exist
    if (subpassDependencies.size() > 0){
        renderPassCreateInfo.dependencyCount = subpassDependencyCount;
        renderPassCreateInfo.pDependencies   = &subpassDependencies[0];
    }

    assert(deviceContext->vkCreateRenderPass(deviceContext->device, &renderPassCreateInfo, nullptr, &renderPass) == VK_SUCCESS);
}

VulkanRenderPass::~VulkanRenderPass(){
    deviceContext->vkDestroyRenderPass(deviceContext->device, renderPass, nullptr);
}