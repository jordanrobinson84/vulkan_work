#include "vulkanRenderPass.h"

VulkanRenderPass::VulkanRenderPass(VulkanDevice                          * __deviceContext,
                                   std::vector<VkAttachmentDescription>  & attachments,
                                   std::vector<VkSubpassDescription>     & subpasses,
                                   std::vector<VkSubpassDependency>      & subpassDependecies = {}){
    deviceContext = __deviceContext;
    assert(deviceContext != nullptr);
    assert(attachments.size() > 0);
    assert(subpasses.size() > 0);

    VkRenderPassCreateInfo renderPassCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        attachments.size(),
        &attachments[0],
        subpasses.size(),
        &subpasses[0],
        0,
        nullptr
    };

    // Add subpasses if they exist
    if (subpassDependecies.size() > 0){
        renderPassCreateInfo.dependencyCount = subpassDependecies.size();
        renderPassCreateInfo.pDependencies   = &subpassDependecies[0];
    }
    VkRenderPass renderPass;
    assert(deviceContext->vkCreateRenderPass(deviceContext->device, &renderPassCreateInfo, nullptr, &renderPass) == VK_SUCCESS);
}

VulkanRenderPass::~VulkanRenderPass(){
    deviceContext->VkDestroyRenderPass(deviceContext->device, renderPass, nullptr);
}