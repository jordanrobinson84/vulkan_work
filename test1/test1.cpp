#include <iostream>
#include <cassert>
#include "vulkanDriverInstance.h"
#include "vulkanBuffer.h"
#include "vulkanRenderPass.h"
#include "vulkanSwapchain.h"
#include "vulkanPipelineState.h"

#if defined (_WIN32) || defined (_WIN64)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow){
    VulkanDriverInstance instance("Windows");
#elif defined (__linux__)
int main(){
    VulkanDriverInstance instance("Linux");
#endif

    if(instance.loader == nullptr){
        std::cout << "Vulkan library not found!" << std::endl;
        return false;
    }
    std::cout << "Vulkan library found!" << std::endl;
    if (instance.vkGetInstanceProcAddr != nullptr){
        std::cout << "vkGetInstanceProcAddr function found!" << std::endl;
    }
    if (instance.vkCreateInstance != nullptr){
        std::cout << "vkCreateInstance function found!" << std::endl;
    }

    // Set up instance variables and functions
    VulkanDevice * deviceContext = new VulkanDevice(&instance, 0); // Create device for device #0
    if (deviceContext == nullptr){
        std::cout << "Could not create a Vulkan Device!" << std:: endl;
        return false;
    }

    // Set buffer data
    float vertexBufferData[18];
    vertexBufferData[0]  = 0.2; // x[0]
    vertexBufferData[1]  = 0.2; // y[0]
    vertexBufferData[2]  = 0.0; // z[0]
    vertexBufferData[3]  = 0.2; // r[0]
    vertexBufferData[4]  = 0.2; // g[0]
    vertexBufferData[5]  = 0.0; // b[0]
    vertexBufferData[6]  = 0.2; // x[1]
    vertexBufferData[7]  = 0.8; // y[1]
    vertexBufferData[8]  = 0.0; // z[1]
    vertexBufferData[9]  = 0.2; // r[1]
    vertexBufferData[10] = 0.8; // g[1]
    vertexBufferData[11] = 0.0; // b[1]
    vertexBufferData[12] = 0.8; // x[2]
    vertexBufferData[13] = 0.5; // y[2]
    vertexBufferData[14] = 0.0; // z[2]
    vertexBufferData[15] = 0.8; // r[2]
    vertexBufferData[16] = 0.5; // g[2]
    vertexBufferData[17] = 0.0; // b[2]

    VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferData, sizeof(float) * 18, false);
    const VkDeviceSize vertexOffset = 0;
    
    // Window geometry
    const uint32_t windowWidth  = 512;
    const uint32_t windowHeight = 512;

    std::vector<uint32_t> queueFamilyIndices = {0};
    VulkanSwapchain swapchain(&instance, deviceContext, instance.physicalDevices[0], queueFamilyIndices);

#if defined (_WIN32) || defined (_WIN64)
    swapchain.createWindow(hInstance, windowWidth, windowHeight);
#elif defined (__linux__)
    swapchain.createWindow(windowWidth, windowHeight);
#endif

    VulkanPipelineState vps(deviceContext);

    VkVertexInputBindingDescription vertexBindingDescription;
    vertexBindingDescription.binding    = 0;
    vertexBindingDescription.stride     = 6 * sizeof(float);
    vertexBindingDescription.inputRate  = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription positionInputDescription;
    positionInputDescription.binding    = 0;
    positionInputDescription.format     = VK_FORMAT_R32G32B32_SFLOAT;
    positionInputDescription.location   = 0;
    positionInputDescription.offset     = 0;

    VkVertexInputAttributeDescription colorInputDescription;
    colorInputDescription.binding   = 0;
    colorInputDescription.format    = VK_FORMAT_R32G32B32_SFLOAT;
    colorInputDescription.location  = 1;
    colorInputDescription.offset    = 3 * sizeof(float);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions     = { vertexBindingDescription };
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = { positionInputDescription, colorInputDescription };
    vps.setPrimitiveState(bindingDescriptions, attributeDescriptions, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkRect2D scissorRect = { { 0, 0 }, swapchain.extent };
    vps.setViewportState(swapchain.extent, scissorRect);

    // Do rendering
    VulkanCommandPool * renderPool      = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndices[0]);
    VkCommandBuffer * cmdBuffer         = renderPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 3);

    // Create render pass
    VkAttachmentDescription defaultAttachment = {
        VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
        swapchain.surfaceFormats[0].format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpassDescription = {
        0, // Flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        0, // inputAttachmentCount
        nullptr, // pInputAttachments
        1, // colorAttachmentCount
        &colorAttachment, // pColorAttachments
        nullptr, // pResolveAttachments
        nullptr, // pDepthStencilAttachment
        0, // preserveAttachmentCount
        nullptr// pPreserveAttachments
    };

    std::vector<VkAttachmentDescription> attachments = {defaultAttachment};
    std::vector<VkSubpassDescription> subpasses ={subpassDescription};
    std::vector<VkSubpassDependency> dependencies = {};
    VulkanRenderPass rp(deviceContext, attachments, subpasses, dependencies);

    // Blend states
    VkPipelineColorBlendAttachmentState attachmentBlendState;
    attachmentBlendState.blendEnable            = VK_FALSE;
    attachmentBlendState.srcColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.colorBlendOp           = VK_BLEND_OP_ADD;
    attachmentBlendState.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
    attachmentBlendState.alphaBlendOp           = VK_BLEND_OP_ADD;
    attachmentBlendState.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blendState;
    blendState.sType                = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.pNext                = nullptr;
    blendState.flags                = 0; //MBZ
    blendState.logicOpEnable        = VK_FALSE;
    blendState.logicOp              = VK_LOGIC_OP_NO_OP;
    blendState.attachmentCount      = subpasses[0].colorAttachmentCount;
    blendState.pAttachments         = &attachmentBlendState;
    blendState.blendConstants[0]    = 1.0;
    blendState.blendConstants[1]    = 1.0;
    blendState.blendConstants[2]    = 1.0;
    blendState.blendConstants[3]    = 1.0;
    vps.pipelineInfo.pColorBlendState = &blendState;

    vps.pipelineInfo.renderPass = rp.renderPass;

    // Pipeline layout setup
    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext                    = nullptr;
    layoutInfo.flags                    = 0;
    layoutInfo.setLayoutCount           = 0;        // No descriptor sets enabled
    layoutInfo.pSetLayouts              = nullptr;  // No descriptor sets enabled
    layoutInfo.pushConstantRangeCount   = 0;
    layoutInfo.pPushConstantRanges      = nullptr;
    assert(deviceContext->vkCreatePipelineLayout(deviceContext->device, &layoutInfo, nullptr, &layout) == VK_SUCCESS);
    vps.pipelineInfo.layout = layout;

	vps.addShaderStage("vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
	vps.addShaderStage("frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
    vps.complete();

    VkCommandBufferBeginInfo cmdBufferBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };
    
    std::vector<VkFence> submitFences;
    submitFences.resize(3);
    VkFenceCreateInfo submitFenceInfo;
    submitFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    submitFenceInfo.pNext = nullptr;
    submitFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (int i = 0; i < 3; i++) {
        assert(deviceContext->vkCreateFence(deviceContext->device, &submitFenceInfo, nullptr, &submitFences[i]) == VK_SUCCESS);
    }

    assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[0], &cmdBufferBeginInfo) == VK_SUCCESS);
    swapchain.setupFramebuffers(cmdBuffer[0], rp.renderPass);
    // swapchain.setImageLayout(cmdBuffer[0], swapchain.getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); 

    uint32_t frameCount = 0;
    VkQueue presentQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, queueFamilyIndices[0], 0, &presentQueue);
    char loopChar = ' ';

    while(loopChar != '\r'){
        int cmdBufferIndex = frameCount % 3;
        int nextCmdBufferIndex = (frameCount + 1) % 3;

        // Begin the render pass
        VkClearValue colorClear = {0.0f, 1.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo renderPassBegin = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            rp.renderPass,
            swapchain.getCurrentFramebuffer(),
            {{0,0}, swapchain.extent},
            1,
            &colorClear
        };
        deviceContext->vkCmdBindPipeline(cmdBuffer[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vps.pipeline);
        deviceContext->vkCmdBeginRenderPass(cmdBuffer[cmdBufferIndex], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
        deviceContext->vkCmdBindVertexBuffers(cmdBuffer[cmdBufferIndex], 0, 1, &vertexBuffer.bufferHandle, &vertexOffset);
        deviceContext->vkCmdDraw(cmdBuffer[cmdBufferIndex], 3, 1, 0, 0);

        // Dispatch
        /*VkQueue presentQueue;
        deviceContext->vkGetDeviceQueue(deviceContext->device, queueFamilyIndices[0], 0, &presentQueue);*/
        const VkPipelineStageFlags stageFlags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo presentSubmitInfo;
        presentSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        presentSubmitInfo.pNext                = nullptr;
        presentSubmitInfo.waitSemaphoreCount   = 1;
        presentSubmitInfo.pWaitSemaphores      = &swapchain.presentationSemaphore;
        presentSubmitInfo.pWaitDstStageMask    = &stageFlags[0];
        presentSubmitInfo.commandBufferCount   = 1;
        presentSubmitInfo.pCommandBuffers      = &cmdBuffer[cmdBufferIndex];
        presentSubmitInfo.signalSemaphoreCount = 1;
        presentSubmitInfo.pSignalSemaphores    = &swapchain.renderingDoneSemaphore;

        // End render pass
        deviceContext->vkCmdEndRenderPass(cmdBuffer[cmdBufferIndex]);
        swapchain.setImageLayout(cmdBuffer[cmdBufferIndex], swapchain.getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        deviceContext->vkEndCommandBuffer(cmdBuffer[cmdBufferIndex]);
        assert(deviceContext->vkQueueSubmit(presentQueue, 1, &presentSubmitInfo, submitFences[cmdBufferIndex]) == VK_SUCCESS);

        // Present
        swapchain.present(presentQueue);
        deviceContext->vkWaitForFences(deviceContext->device, 1, &submitFences[nextCmdBufferIndex], VK_TRUE, 0x1000000);
        deviceContext->vkResetFences(deviceContext->device, 1, &submitFences[nextCmdBufferIndex]);
        renderPool->resetCommandBuffer(cmdBuffer[nextCmdBufferIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        frameCount++;
        // if (frameCount < 300){
            assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[nextCmdBufferIndex], &cmdBufferBeginInfo) == VK_SUCCESS);
        // }
        loopChar = std::cin.get();
    }

    // Wait
    // std::cin.get();
    assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
    renderPool->freeCommandBuffers(3, cmdBuffer);
    delete renderPool;

    return 0;
}
