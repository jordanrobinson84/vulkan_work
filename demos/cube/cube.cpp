#include <iostream>
#include <cassert>
#include <chrono>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#include "vulkanDriverInstance.h"
#include "vulkanBuffer.h"
#include "vulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "vulkanPipelineState.h"

struct uniformLayoutStruct{
    glm::mat4 MVP;
    glm::mat4 Normal;
};

#if defined (_WIN32) || defined (_WIN64)
#include "win32Window.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow){
    VulkanDriverInstance instance("Windows");
#elif defined (__linux__)
#include "xcbWindow.h"
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

    // Create Model-View-Projection matrix
    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 1.0f, 0.1f, 100.f);
    glm::mat4 View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 Model = glm::scale(glm::mat4(), glm::vec3(0.5f));

    // Set buffer data
    uint32_t numVertices = 36;
    uint32_t numElements = numVertices * 3;
    glm::vec3 cubeBufferData[108];
    // Red
    cubeBufferData[0] = glm::vec3(-1.0f, -1.0f, 1.0f);
    cubeBufferData[1] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[2] = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[3] = glm::vec3(-1.0f, 1.0f, 1.0f);
    cubeBufferData[4] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[5] = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[6] = glm::vec3(1.0f, -1.0f, 1.0f);
    cubeBufferData[7] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[8] = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[9] = glm::vec3(1.0f, -1.0f, 1.0f);
    cubeBufferData[10] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[11] = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[12] = glm::vec3(-1.0f, 1.0f, 1.0f);
    cubeBufferData[13] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[14] = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[15] = glm::vec3(1.0f, 1.0f, 1.0f);
    cubeBufferData[16] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[17] = glm::vec3(0.0f, 0.0f, -1.0f);

    // Green
    cubeBufferData[18] = glm::vec3(-1.0f, -1.0f, -1.0f);
    cubeBufferData[19] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[20] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[21] = glm::vec3(1.0f, -1.0f, -1.0f);
    cubeBufferData[22] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[23] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[24] = glm::vec3(-1.0f, 1.0f, -1.0f);
    cubeBufferData[25] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[26] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[27] = glm::vec3(-1.0f, 1.0f, -1.0f);
    cubeBufferData[28] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[29] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[30] = glm::vec3(1.0f, -1.0f, -1.0f);
    cubeBufferData[31] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[32] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[33] = glm::vec3(1.0f, 1.0f, -1.0f);
    cubeBufferData[34] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[35] = glm::vec3(0.0f, 0.0f, 1.0f);

    // Blue
    cubeBufferData[36] = glm::vec3(-1.0f, 1.0f, 1.0f);
    cubeBufferData[37] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[38] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[39] = glm::vec3(-1.0f, -1.0f, 1.0f);
    cubeBufferData[40] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[41] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[42] = glm::vec3(-1.0f, 1.0f, -1.0f);
    cubeBufferData[43] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[44] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[45] = glm::vec3(-1.0f, 1.0f, -1.0f);
    cubeBufferData[46] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[47] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[48] = glm::vec3(-1.0f, -1.0f, 1.0f);
    cubeBufferData[49] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[50] = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[51] = glm::vec3(-1.0f, -1.0f, -1.0f);
    cubeBufferData[52] = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[53] = glm::vec3(1.0f, 0.0f, 0.0f);

    // Yellow
    cubeBufferData[54] = glm::vec3(1.0f, 1.0f, 1.0f);
    cubeBufferData[55] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[56] = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[57] = glm::vec3(1.0f, 1.0f, -1.0f);
    cubeBufferData[58] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[59] = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[60] = glm::vec3(1.0f, -1.0f, 1.0f);
    cubeBufferData[61] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[62] = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[63] = glm::vec3(1.0f, -1.0f, 1.0f);
    cubeBufferData[64] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[65] = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[66] = glm::vec3(1.0f, 1.0f, -1.0f);
    cubeBufferData[67] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[68] = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[69] = glm::vec3(1.0f, -1.0f, -1.0f);
    cubeBufferData[70] = glm::vec3(1.0f, 1.0f, 0.0f);
    cubeBufferData[71] = glm::vec3(-1.0f, 1.0f, 0.0f);

    // Teal
    cubeBufferData[72] = glm::vec3(1.0f, 1.0f, 1.0f);
    cubeBufferData[73] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[74] = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[75] = glm::vec3(-1.0f, 1.0f, 1.0f);
    cubeBufferData[76] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[77] = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[78] = glm::vec3(1.0f, 1.0f, -1.0f);
    cubeBufferData[79] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[80] = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[81] = glm::vec3(1.0f, 1.0f, -1.0f);
    cubeBufferData[82] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[83] = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[84] = glm::vec3(-1.0f, 1.0f, 1.0f);
    cubeBufferData[85] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[86] = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[87] = glm::vec3(-1.0f, 1.0f, -1.0f);
    cubeBufferData[88] = glm::vec3(0.0f, 1.0f, 1.0f);
    cubeBufferData[89] = glm::vec3(0.0f, -1.0f, 0.0f);

    // Magenta
    cubeBufferData[90] = glm::vec3(1.0f, -1.0f, -1.0f);
    cubeBufferData[91] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[92] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[93] = glm::vec3(-1.0f, -1.0f, 1.0f);
    cubeBufferData[94] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[95] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[96] = glm::vec3(1.0f, -1.0f, 1.0f);
    cubeBufferData[97] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[98] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[99] = glm::vec3(-1.0f, -1.0f, 1.0f);
    cubeBufferData[100] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[101] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[102] = glm::vec3(1.0f, -1.0f, -1.0f);
    cubeBufferData[103] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[104] = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[105] = glm::vec3(-1.0f, -1.0f, -1.0f);
    cubeBufferData[106] = glm::vec3(1.0f, 0.0f, 1.0f);
    cubeBufferData[107] = glm::vec3(0.0f, 1.0f, 0.0f);

    uniformLayoutStruct uniformStruct;
    uniformStruct.MVP = Projection * View * Model;
    uniformStruct.Normal = glm::transpose(glm::inverse(View * Model));
    VulkanBuffer matrixBuffer(deviceContext, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uniformStruct, sizeof(uniformStruct), true);

    // Create Descriptor Set Layout
    VkDescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding            = 0;
    uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount    = 1;
    uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayout uboLayout;
    VkDescriptorSetLayoutCreateInfo uboLayoutInfo;
    uboLayoutInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboLayoutInfo.pNext         = nullptr;
    uboLayoutInfo.flags         = 0;
    uboLayoutInfo.bindingCount  = 1;
    uboLayoutInfo.pBindings     = &uboLayoutBinding;

    // // Create Descriptor Pool
    VkDescriptorPoolSize uniformPoolSize;
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = 1;
    std::vector<VkDescriptorPoolSize> poolSizes = {uniformPoolSize};

    VkDescriptorPool *  uniformPool  = deviceContext->getDescriptorPool(poolSizes);

    assert(deviceContext->vkCreateDescriptorSetLayout(deviceContext->device, &uboLayoutInfo, nullptr, &uboLayout) == VK_SUCCESS);

    // Allocate Descriptor Set
    VkDescriptorSetAllocateInfo uniformSetInfo;
    uniformSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uniformSetInfo.pNext = nullptr;
    uniformSetInfo.descriptorPool = *uniformPool;
    uniformSetInfo.descriptorSetCount = 1;
    uniformSetInfo.pSetLayouts = &uboLayout;

    VkDescriptorSet uniformSet;
    assert(deviceContext->vkAllocateDescriptorSets(deviceContext->device, &uniformSetInfo, &uniformSet) == VK_SUCCESS);

    // Associate Uniform Buffer with Descriptor Set
    VkDescriptorBufferInfo uniformBufferInfo;
    uniformBufferInfo.buffer = matrixBuffer.bufferHandle;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = matrixBuffer.payloadSize;

    VkWriteDescriptorSet uniformWriteSet;
    uniformWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniformWriteSet.pNext = nullptr;
    uniformWriteSet.dstSet = uniformSet;
    uniformWriteSet.dstBinding = 0;
    uniformWriteSet.dstArrayElement = 0;
    uniformWriteSet.descriptorCount = 1;
    uniformWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformWriteSet.pImageInfo = nullptr;
    uniformWriteSet.pBufferInfo = &uniformBufferInfo;
    uniformWriteSet.pTexelBufferView = nullptr;

    deviceContext->vkUpdateDescriptorSets(deviceContext->device, 1, &uniformWriteSet, 0, nullptr);

    VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, cubeBufferData, sizeof(glm::vec3) * numElements, false);
    const VkDeviceSize vertexOffset = 0;
    
    // Window geometry
    const uint32_t windowWidth  = 512;
    const uint32_t windowHeight = 512;

    Window * window = nullptr;

#if defined (_WIN32) || defined (_WIN64)
    window = new Win32Window(windowWidth, windowHeight, &instance, deviceContext, instance.physicalDevices[0], "Win32 Vulkan - Test1");
#elif defined (__linux__)
    window = new XcbWindow(windowWidth, windowHeight, &instance, deviceContext, instance.physicalDevices[0], "Linux XCB Vulkan - Test1");
#endif

    VulkanPipelineState vps(deviceContext);

    VkVertexInputBindingDescription vertexBindingDescription;
    vertexBindingDescription.binding    = 0;
    vertexBindingDescription.stride     = 3 * sizeof(glm::vec3);
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
    colorInputDescription.offset    = sizeof(glm::vec3);

    VkVertexInputAttributeDescription normalInputDescription;
    normalInputDescription.binding   = 0;
    normalInputDescription.format    = VK_FORMAT_R32G32B32_SFLOAT;
    normalInputDescription.location  = 2;
    normalInputDescription.offset    = 2 * sizeof(glm::vec3);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions     = { vertexBindingDescription };
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = { positionInputDescription, colorInputDescription, normalInputDescription };
    vps.setPrimitiveState(bindingDescriptions, attributeDescriptions, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkRect2D scissorRect = { { 0, 0 }, window->swapchain->extent };
    vps.setViewportState(window->swapchain->extent, scissorRect);
    window->swapchain->setPipelineState(&vps);

    // Do rendering
    VulkanCommandPool * renderPool      = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, window->swapchain->queueFamilyIndices[0]);
    VkCommandBuffer * cmdBuffer         = renderPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, window->swapchain->imageCount);

    window->swapchain->createRenderpass();

    // Pipeline layout setup
    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext                    = nullptr;
    layoutInfo.flags                    = 0;
    layoutInfo.setLayoutCount           = 1;
    layoutInfo.pSetLayouts              = &uboLayout;
    layoutInfo.pushConstantRangeCount   = 0;
    layoutInfo.pPushConstantRanges      = nullptr;
    assert(deviceContext->vkCreatePipelineLayout(deviceContext->device, &layoutInfo, nullptr, &layout) == VK_SUCCESS);
    vps.pipelineInfo.layout = layout;

    vps.addShaderStage("vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    vps.addShaderStage("frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
    vps.complete();

    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    cmdBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext            = nullptr;
    cmdBufferBeginInfo.flags            = 0;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;
    
    // Set up submit fences
    std::vector<VkFence> submitFences;
    submitFences.resize(window->swapchain->imageCount);
    
    for (int i = 0; i < window->swapchain->imageCount; i++) {
        VkFenceCreateInfo submitFenceInfo;
        submitFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        submitFenceInfo.pNext = nullptr;
        submitFenceInfo.flags = 0; // Unsignaled state
        assert(deviceContext->vkCreateFence(deviceContext->device, &submitFenceInfo, nullptr, &submitFences[i]) == VK_SUCCESS);
    }

    assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[0], &cmdBufferBeginInfo) == VK_SUCCESS);
    // window->swapchain->setupFramebuffers(cmdBuffer[0]);
    // window->swapchain->setImageLayout(cmdBuffer[0], window->swapchain->getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); 

    uint32_t frameCount = 0;
    VkQueue presentQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, window->swapchain->queueFamilyIndices[0], 0, &presentQueue);

    window->swapchain->setupFramebuffers(cmdBuffer[0]);

    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
    end = start;

    while(true){
        int cmdBufferIndex = frameCount % window->swapchain->imageCount;
        int nextCmdBufferIndex = (frameCount + 1) % window->swapchain->imageCount;

        if(window->swapchain->dirtyFramebuffers){
            // Reset command buffers
            for(int i = 0; i < window->swapchain->imageCount; i++){
                renderPool->resetCommandBuffer(cmdBuffer[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            }

            std::cout << "Recreating Framebuffers" << std::endl;
            assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
            assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[cmdBufferIndex], &cmdBufferBeginInfo) == VK_SUCCESS);
            window->swapchain->setupFramebuffers(cmdBuffer[cmdBufferIndex]);

            // Update Projection Matrix
            float fWidth = (float)window->swapchain->extent.width;
            float fHeight = (float)window->swapchain->extent.height;
            float aspect = fWidth / fHeight;
            
            std::cout << "New Aspect Ratio: " << aspect << std::endl;
            if(std::isfinite(aspect)){
                Projection = glm::perspective(glm::pi<float>() * 0.25f, aspect, 0.1f, 100.f);
            }
        }

        // Update uniform buffer if duration is greater than zero
        if(start != end){
            std::chrono::duration<double, std::ratio<1, 1000>> delta_time = end - start;
            double delta = delta_time.count();
            float angle = (float)(glm::pi<double>() * 0.5 * (delta / 1000.0));

            Model = glm::rotate(Model, angle, glm::vec3(0.0f, 0.4f, 1.0f));
            uniformStruct.MVP = Projection * View * Model;
            uniformStruct.Normal = glm::transpose(glm::inverse(View * Model));
            void * mapData = nullptr;
            assert( deviceContext->vkMapMemory(deviceContext->device, matrixBuffer.bufferMemory, 0, matrixBuffer.memoryRequirements.size, 0, &mapData) == VK_SUCCESS);
            memcpy(mapData, &uniformStruct, sizeof(uniformStruct));
            deviceContext->vkUnmapMemory(deviceContext->device, matrixBuffer.bufferMemory);
            start = end;
        }

        // Begin the render pass
        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBegin;
        renderPassBegin.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBegin.pNext           = nullptr;
        renderPassBegin.renderPass      = window->swapchain->renderPass;
        renderPassBegin.framebuffer     = window->swapchain->getCurrentFramebuffer();
        renderPassBegin.renderArea      = {{0,0}, window->swapchain->extent};
        renderPassBegin.clearValueCount = clearValues.size();
        renderPassBegin.pClearValues    = &clearValues[0];

        deviceContext->vkQueueWaitIdle(presentQueue);
        deviceContext->vkCmdBindDescriptorSets(cmdBuffer[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &uniformSet, 0, nullptr);
        deviceContext->vkCmdBindPipeline(cmdBuffer[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vps.pipeline);
        deviceContext->vkCmdBeginRenderPass(cmdBuffer[cmdBufferIndex], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
        deviceContext->vkCmdBindVertexBuffers(cmdBuffer[cmdBufferIndex], 0, 1, &vertexBuffer.bufferHandle, &vertexOffset);
        deviceContext->vkCmdDraw(cmdBuffer[cmdBufferIndex], numVertices, 1, 0, 0);

        // Dispatch
        /*VkQueue presentQueue;
        deviceContext->vkGetDeviceQueue(deviceContext->device, queueFamilyIndices[0], 0, &presentQueue);*/
        const VkPipelineStageFlags stageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo presentSubmitInfo;
        presentSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        presentSubmitInfo.pNext                = nullptr;
        presentSubmitInfo.waitSemaphoreCount   = 1;
        presentSubmitInfo.pWaitSemaphores      = &window->swapchain->presentationSemaphore;
        presentSubmitInfo.pWaitDstStageMask    = stageFlags;
        presentSubmitInfo.commandBufferCount   = 1;
        presentSubmitInfo.pCommandBuffers      = &cmdBuffer[cmdBufferIndex];
        presentSubmitInfo.signalSemaphoreCount = 1;
        presentSubmitInfo.pSignalSemaphores    = &window->swapchain->renderingDoneSemaphore;

        // End render pass
        deviceContext->vkCmdEndRenderPass(cmdBuffer[cmdBufferIndex]);
        window->swapchain->setImageLayout(cmdBuffer[cmdBufferIndex], window->swapchain->getCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        deviceContext->vkEndCommandBuffer(cmdBuffer[cmdBufferIndex]);
        assert(deviceContext->vkQueueSubmit(presentQueue, 1, &presentSubmitInfo, submitFences[cmdBufferIndex]) == VK_SUCCESS);
        // assert(deviceContext->vkQueueSubmit(presentQueue, 1, &presentSubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);

        // Present
        try{
            window->swapchain->present(presentQueue);
        }catch(std::runtime_error err){
            std::cout << "Present error: " << err.what() << std::endl;
        }

        // Update timer
        end = std::chrono::high_resolution_clock::now();

        // deviceContext->vkWaitForFences(deviceContext->device, 1, &submitFences[nextCmdBufferIndex], VK_TRUE, 0x1000000);
        // deviceContext->vkResetFences(deviceContext->device, 1, &submitFences[nextCmdBufferIndex]);
        if (frameCount >= window->swapchain->imageCount-1){
            deviceContext->vkResetFences(deviceContext->device, 1, &submitFences[nextCmdBufferIndex]);
            renderPool->resetCommandBuffer(cmdBuffer[nextCmdBufferIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }
        frameCount = (frameCount + 1) % (std::numeric_limits<uint32_t>::max)();
        assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[nextCmdBufferIndex], &cmdBufferBeginInfo) == VK_SUCCESS);

        // std::cout << "Frame #" << frameCount << std::endl;

        #if defined (_WIN32) || defined (_WIN64)
            MSG message;

            while( PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)){
                TranslateMessage(&message);
                DispatchMessage(&message);  
            }
        #endif
    }

    // Wait
    // std::cin.get();
    assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
    renderPool->freeCommandBuffers(window->swapchain->imageCount, cmdBuffer);
    delete renderPool;

    return 0;
}
