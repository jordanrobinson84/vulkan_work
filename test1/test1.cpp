#include <iostream>
#include <cassert>
#include "vulkanDriverInstance.h"
#include "vulkanBuffer.h"
#include "vulkanRenderPass.h"
#include "vulkanSwapchain.h"

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
    float vertexBufferData[9];
    vertexBufferData[0] = 0.2; // x[0]
    vertexBufferData[1] = 0.2; // y[0]
    vertexBufferData[2] = 0.0; // z[0]
    vertexBufferData[3] = 0.2; // x[1]
    vertexBufferData[4] = 0.8; // y[1]
    vertexBufferData[5] = 0.0; // z[1]
    vertexBufferData[6] = 0.8; // x[2]
    vertexBufferData[7] = 0.5; // y[2]
    vertexBufferData[8] = 0.0; // z[2]

    // VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferData, sizeof(float) * 9, false);
    
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

    // VkRenderPassCreateInfo renderPassCreateInfo = {
    //     VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    //     nullptr,
    //     0,
    //     1,
    //     &defaultAttachment,
    //     1,
    //     &subpassDescription, // Need to fill in subpass
    //     0,
    //     nullptr
    // };
    // VkRenderPass renderPass;
    // assert(deviceContext->vkCreateRenderPass(deviceContext->device, &renderPassCreateInfo, nullptr, &renderPass) == VK_SUCCESS);
    std::vector<VkAttachmentDescription> attachments = {defaultAttachment};
    std::vector<VkSubpassDescription> subpasses ={subpassDescription};
    std::vector<VkSubpassDependency> dependencies = {};
    VulkanRenderPass rp(deviceContext, attachments, subpasses, dependencies);

    VkCommandBufferBeginInfo cmdBufferBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };
    assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[0], &cmdBufferBeginInfo) == VK_SUCCESS);
    swapchain.setupFramebuffers(cmdBuffer[0], rp.renderPass);
    // swapchain.setImageLayout(cmdBuffer[0], swapchain.getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); 

    uint32_t frameCount = 0;

    while(frameCount < 2){

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
        deviceContext->vkCmdBeginRenderPass(cmdBuffer[frameCount], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

        // Dispatch
        VkQueue presentQueue;
        deviceContext->vkGetDeviceQueue(deviceContext->device, queueFamilyIndices[0], 0, &presentQueue);
        const VkPipelineStageFlags stageFlags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo presentSubmitInfo;
        presentSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        presentSubmitInfo.pNext                = nullptr;
        presentSubmitInfo.waitSemaphoreCount   = 1;
        presentSubmitInfo.pWaitSemaphores      = &swapchain.presentationSemaphore;
        presentSubmitInfo.pWaitDstStageMask    = &stageFlags[0];
        presentSubmitInfo.commandBufferCount   = 1;
        presentSubmitInfo.pCommandBuffers      = &cmdBuffer[frameCount];
        presentSubmitInfo.signalSemaphoreCount = 1;
        presentSubmitInfo.pSignalSemaphores    = &swapchain.renderingDoneSemaphore;

        // End render pass
        deviceContext->vkCmdEndRenderPass(cmdBuffer[frameCount]);
        swapchain.setImageLayout(cmdBuffer[frameCount], swapchain.getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        deviceContext->vkEndCommandBuffer(cmdBuffer[frameCount]);
        assert(deviceContext->vkQueueSubmit(presentQueue, 1, &presentSubmitInfo, 0) == VK_SUCCESS);

        // Present
        swapchain.present(presentQueue);
        // renderPool->resetCommandBuffer(cmdBuffer[frameCount], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        frameCount++;
        if (frameCount < 3){
            assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[frameCount], &cmdBufferBeginInfo) == VK_SUCCESS);
        }
    }

    // Wait
    std::cin.get();
    renderPool->freeCommandBuffers(1, cmdBuffer);
    delete renderPool;
    // deviceContext->vkDestroyRenderPass(deviceContext->device, renderPass, nullptr);

    return 0;
}
