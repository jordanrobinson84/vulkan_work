#include <iostream>
#include <cassert>
#include <chrono>
#include <regex>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>
#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb/stb_image.h>
#endif
#include "VulkanDriverInstance.h"
#include "VulkanBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "VulkanPipelineState.h"

struct uniformLayoutStruct{
    glm::mat4 MVP;
    glm::mat4 Normal;
};

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};

#define ROTATION_RATE 0.5
#define VERTICAL_FOV 0.25
#define MILLISECONDS_TO_SECONDS 1000
#define FRAME_RATE_UPDATE_INTERVAL 5

#if defined (_WIN32) || defined (_WIN64)
#include "win32Window.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow){
    VulkanDriverInstance instance("Windows");

    // Get command line arguments
    LPWSTR* argv;
    int argc;
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#elif defined (__linux__)
#include "xcbWindow.h"
int main(int argc, char **argv){
    VulkanDriverInstance instance("Linux");
#endif

    // Only check for multisample command line option if at least one argument is specified
    uint32_t sampleCount = 1;
    VkSampleCountFlagBits sampleCountFlag;
    if(argc > 1){
        std::string multisampleCmdLine;
#if defined (_WIN32) || defined (_WIN64)
        uint32_t multisampleCmdLineSize = wcslen(argv[1]);
        multisampleCmdLine = std::string(multisampleCmdLineSize, ' ');
        wcstombs(&multisampleCmdLine[0], argv[1], multisampleCmdLineSize);
#elif defined (__linux__)
        multisampleCmdLine = std::string(argv[1]);
        std::cout << "Multisample command line: " << multisampleCmdLine << std::endl;
#endif
        std::regex msaaRegex("MSAASampleCount=([0-9]+)");
        std::smatch matches;
        std::string sampleCountString;

        if(std::regex_match(multisampleCmdLine, matches, msaaRegex)){
            try{
                sampleCountString = (matches[1].str)();
                sampleCount = std::stoul(sampleCountString);
                std::cout << "Sample count \"" << sampleCountString << "\" specified." << std::endl;
            }catch(std::out_of_range oor){
                std::cout << "Invalid sample count \"" << sampleCountString << "\" specified, the proper usage is \"MSAASampleCount=<count>\"." << std::endl;
                sampleCount = 1;
            }catch(std::invalid_argument ia){
                std::cout << "Invalid sample count \"" << sampleCountString << "\" specified, the proper usage is \"MSAASampleCount=<count>\"." << std::endl;
                sampleCount = 1;
            }
        }
    }

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
    VkPhysicalDeviceFeatures requiredFeatures = {VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE, VK_FALSE};
    VkPhysicalDeviceFeatures requestedFeatures = {VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE, VK_TRUE};
    requiredFeatures.sampleRateShading = VK_TRUE;
    requiredFeatures.samplerAnisotropy = VK_TRUE;
    VulkanDevice * deviceContext = new VulkanDevice(&instance, 0, &requestedFeatures, &requiredFeatures); // Create device for device #0
    if (deviceContext == nullptr){
        std::cout << "Could not create a Vulkan Device!" << std:: endl;
        return false;
    }

    sampleCountFlag = deviceContext->requestSupportedSampleFlags(sampleCount);

    // Create Model-View-Projection matrix
    glm::mat4 Projection    = glm::perspective(glm::pi<float>() * 0.25f, 1.0f, 0.1f, 100.f);
    glm::mat4 View          = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 Model         = glm::scale(glm::mat4(), glm::vec3(0.5f));

    // Set buffer data
    uint32_t numVertices = 24;
    uint32_t numElements = numVertices * 3;
    uint32_t numIndices = 36;
    Vertex cubeBufferData[24];
    uint16_t cubeIndexBufferData[36] = {0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23};
    // Red
    cubeBufferData[0].position  = glm::vec3(-1.0f, -1.0f, 1.0f); // [0]
    cubeBufferData[0].normal    = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[0].texcoords = glm::vec2(0.0f, 0.0f);
    cubeBufferData[1].position  = glm::vec3(-1.0f, 1.0f, 1.0f); // [1]
    cubeBufferData[1].normal    = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[1].texcoords = glm::vec2(0.0f, 1.0f);
    cubeBufferData[2].position  = glm::vec3(1.0f, -1.0f, 1.0f); // [2]
    cubeBufferData[2].normal    = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[2].texcoords = glm::vec2(1.0f, 0.0f);
    cubeBufferData[3].position  = glm::vec3(1.0f, 1.0f, 1.0f); // [3]
    cubeBufferData[3].normal    = glm::vec3(0.0f, 0.0f, -1.0f);
    cubeBufferData[3].texcoords = glm::vec2(1.0f, 1.0f);

    // Green
    cubeBufferData[4].position  = glm::vec3(-1.0f, -1.0f, -1.0f); // [4]
    cubeBufferData[4].normal    = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[4].texcoords = glm::vec2(0.0f, 0.0f);
    cubeBufferData[5].position  = glm::vec3(1.0f, -1.0f, -1.0f); // [5]
    cubeBufferData[5].normal    = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[5].texcoords = glm::vec2(0.0f, 1.0f);
    cubeBufferData[6].position  = glm::vec3(-1.0f, 1.0f, -1.0f); // [6]
    cubeBufferData[6].normal    = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[6].texcoords = glm::vec2(1.0f, 0.0f);
    cubeBufferData[7].position  = glm::vec3(1.0f, 1.0f, -1.0f); // [7]
    cubeBufferData[7].normal    = glm::vec3(0.0f, 0.0f, 1.0f);
    cubeBufferData[7].texcoords = glm::vec2(1.0f, 1.0f);

    // Blue
    cubeBufferData[8].position      = glm::vec3(-1.0f, 1.0f, 1.0f); // [8]
    cubeBufferData[8].normal        = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[8].texcoords     = glm::vec2(0.0f, 0.0f);
    cubeBufferData[9].position      = glm::vec3(-1.0f, -1.0f, 1.0f); // [9]
    cubeBufferData[9].normal        = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[9].texcoords     = glm::vec2(0.0f, 1.0f);
    cubeBufferData[10].position     = glm::vec3(-1.0f, 1.0f, -1.0f); // [10]
    cubeBufferData[10].normal       = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[10].texcoords    = glm::vec2(1.0f, 0.0f);
    cubeBufferData[11].position     = glm::vec3(-1.0f, -1.0f, -1.0f); // [11]
    cubeBufferData[11].normal       = glm::vec3(1.0f, 0.0f, 0.0f);
    cubeBufferData[11].texcoords    = glm::vec2(1.0f, 1.0f);

    // Yellow
    cubeBufferData[12].position = glm::vec3(1.0f, 1.0f, 1.0f); // [12]
    cubeBufferData[12].normal = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[12].texcoords = glm::vec2(0.0f, 0.0f);
    cubeBufferData[13].position = glm::vec3(1.0f, 1.0f, -1.0f); // [13]
    cubeBufferData[13].normal = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[13].texcoords = glm::vec2(0.0f, 1.0f);
    cubeBufferData[14].position = glm::vec3(1.0f, -1.0f, 1.0f); // [14]
    cubeBufferData[14].normal = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[14].texcoords = glm::vec2(1.0f, 0.0f);
    cubeBufferData[15].position = glm::vec3(1.0f, -1.0f, -1.0f); // [15]
    cubeBufferData[15].normal = glm::vec3(-1.0f, 1.0f, 0.0f);
    cubeBufferData[15].texcoords = glm::vec2(1.0f, 1.0f);

    // Teal
    cubeBufferData[16].position = glm::vec3(1.0f, 1.0f, 1.0f); // [16]
    cubeBufferData[16].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[16].texcoords = glm::vec2(0.0f, 0.0f);
    cubeBufferData[17].position = glm::vec3(-1.0f, 1.0f, 1.0f); // [17]
    cubeBufferData[17].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[17].texcoords = glm::vec2(0.0f, 1.0f);
    cubeBufferData[18].position = glm::vec3(1.0f, 1.0f, -1.0f); // [18]
    cubeBufferData[18].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[18].texcoords = glm::vec2(1.0f, 0.0f);
    cubeBufferData[19].position = glm::vec3(-1.0f, 1.0f, -1.0f); // [19]
    cubeBufferData[19].normal = glm::vec3(0.0f, -1.0f, 0.0f);
    cubeBufferData[19].texcoords = glm::vec2(1.0f, 1.0f);

    // Magenta
    cubeBufferData[20].position = glm::vec3(1.0f, -1.0f, 1.0f); // [20]
    cubeBufferData[20].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[20].texcoords = glm::vec2(0.0f, 0.0f);
    cubeBufferData[21].position = glm::vec3(1.0f, -1.0f, -1.0f); // [21]
    cubeBufferData[21].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[21].texcoords = glm::vec2(0.0f, 1.0f);
    cubeBufferData[22].position = glm::vec3(-1.0f, -1.0f, 1.0f); // [22]
    cubeBufferData[22].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[22].texcoords = glm::vec2(1.0f, 0.0f);
    cubeBufferData[23].position = glm::vec3(-1.0f, -1.0f, -1.0f); // [23]
    cubeBufferData[23].normal = glm::vec3(0.0f, 1.0f, 0.0f);
    cubeBufferData[23].texcoords = glm::vec2(1.0f, 1.0f);

    uniformLayoutStruct uniformStruct;
    uniformStruct.MVP = Projection * View * Model;
    uniformStruct.Normal = glm::transpose(glm::inverse(View * Model));
    VulkanBuffer matrixBuffer(deviceContext, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uniformStruct, sizeof(uniformStruct), true);

    // Create pipeline state
    VulkanPipelineState vps(deviceContext);

    VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, cubeBufferData, sizeof(Vertex) * numVertices, false);
    VulkanBuffer indexBuffer(deviceContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, cubeIndexBufferData, sizeof(uint16_t) * numIndices, false);
    const VkDeviceSize vertexOffset = 0;
    
    // Window geometry
    const uint32_t windowWidth  = 512;
    const uint32_t windowHeight = 512;

    Window * window = nullptr;

#if defined (_WIN32) || defined (_WIN64)
    window = new Win32Window(windowWidth, windowHeight, &instance, deviceContext, instance.physicalDevices[0], "Win32 Vulkan - Cube", sampleCountFlag);
#elif defined (__linux__)
    window = new XcbWindow(windowWidth, windowHeight, &instance, deviceContext, instance.physicalDevices[0], "Linux XCB Vulkan - Cube", sampleCountFlag);
#endif

    // Load image data
    int x, y, numChannels;
    stbi_uc* imageData = stbi_load("blkmarbl.png", &x, &y, &numChannels, STBI_rgb_alpha);
    assert(x > 0);
    assert(y > 0);
    assert(numChannels > 0);
    uint32_t width = (uint32_t)x;
    uint32_t height = (uint32_t)y;
    uint32_t imageSize = (uint32_t)x*y*numChannels;
    bool imageWritten = false;

    VulkanImage cubeImage(deviceContext, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, {width, height, 1});
    VkImageView cubeView = cubeImage.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    cubeImage.loadImageData(imageData, imageSize, {width, height, 1});

    // Create Sampler
    VkSampler sampler;
    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext                   = nullptr;
    samplerInfo.flags                   = 0;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = 16.0f;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 1.0f;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    assert(deviceContext->vkCreateSampler(deviceContext->device, &samplerInfo, nullptr, &sampler) == VK_SUCCESS);

    // Add sample uniform
    VkDescriptorPoolSize samplerPoolSize;
    samplerPoolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = 1;

    // Generate descriptor
    VkDescriptorPool * samplerPool = deviceContext->getDescriptorPool({samplerPoolSize});
    VkDescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.binding            = 0;
    samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount    = 1;
    samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    vps.addDescriptorSetLayoutBindings(0, {samplerLayoutBinding});
    std::vector<VkDescriptorSet> samplerDescriptorVector = vps.generateDescriptorSets(*samplerPool);

    // Write descriptor
    VkDescriptorImageInfo samplerImageInfo;
    samplerImageInfo.sampler        = sampler;
    samplerImageInfo.imageView      = cubeImage.imageViewHandle;
    samplerImageInfo.imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet samplerDescriptorWrite;
    samplerDescriptorWrite.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerDescriptorWrite.pNext            = nullptr;
    samplerDescriptorWrite.dstSet           =  samplerDescriptorVector[0];
    samplerDescriptorWrite.dstBinding       = 0;
    samplerDescriptorWrite.dstArrayElement  = 0;
    samplerDescriptorWrite.descriptorCount  = 1;
    samplerDescriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerDescriptorWrite.pImageInfo       = &samplerImageInfo;
    samplerDescriptorWrite.pBufferInfo      = nullptr;
    samplerDescriptorWrite.pTexelBufferView = nullptr;
    deviceContext->vkUpdateDescriptorSets(deviceContext->device, 1, &samplerDescriptorWrite, 0, nullptr);

    VkVertexInputBindingDescription vertexBindingDescription;
    vertexBindingDescription.binding    = 0;
    vertexBindingDescription.stride     = sizeof(Vertex);
    vertexBindingDescription.inputRate  = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription positionInputDescription;
    positionInputDescription.binding    = 0;
    positionInputDescription.format     = VK_FORMAT_R32G32B32_SFLOAT;
    positionInputDescription.location   = 0;
    positionInputDescription.offset     = 0;

    VkVertexInputAttributeDescription normalInputDescription;
    normalInputDescription.binding   = 0;
    normalInputDescription.format    = VK_FORMAT_R32G32B32_SFLOAT;
    normalInputDescription.location  = 1;
    normalInputDescription.offset    = sizeof(glm::vec3);

    VkVertexInputAttributeDescription texCoordBindingDescription;
    texCoordBindingDescription.binding   = 0;
    texCoordBindingDescription.format    = VK_FORMAT_R32G32_SFLOAT;
    texCoordBindingDescription.location  = 2;
    texCoordBindingDescription.offset    = 2 *sizeof(glm::vec3);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions     = { vertexBindingDescription };
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = { positionInputDescription, normalInputDescription, texCoordBindingDescription };
    vps.setPrimitiveState(bindingDescriptions, attributeDescriptions, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkRect2D scissorRect = { { 0, 0 }, window->swapchain->extent };
    vps.setViewportState(window->swapchain->extent, scissorRect);
    window->swapchain->setPipelineState(&vps);

    // Do rendering
    VulkanCommandPool * renderPool      = deviceContext->getCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, window->swapchain->queueFamilyIndices[0]);
    VkCommandBuffer * cmdBuffer         = renderPool->getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, window->swapchain->imageCount);

    window->swapchain->createRenderpass();

    // Pipeline layout setup
    VkPushConstantRange uboRange;
    uboRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboRange.offset     = 0;
    uboRange.size       = sizeof(uniformStruct);

    VkPipelineLayout layout;
    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext                    = nullptr;
    layoutInfo.flags                    = 0;
    layoutInfo.setLayoutCount           = 1;
    layoutInfo.pSetLayouts              = &vps.descriptorSetLayouts.at(0);
    layoutInfo.pushConstantRangeCount   = 1;
    layoutInfo.pPushConstantRanges      = &uboRange;
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
    
    for (uint32_t i = 0; i < window->swapchain->imageCount; i++) {
        VkFenceCreateInfo submitFenceInfo;
        submitFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        submitFenceInfo.pNext = nullptr;
        submitFenceInfo.flags = 0; // Unsignaled state
        assert(deviceContext->vkCreateFence(deviceContext->device, &submitFenceInfo, nullptr, &submitFences[i]) == VK_SUCCESS);
    }

    assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[0], &cmdBufferBeginInfo) == VK_SUCCESS);

    uint32_t frameCount = 0;
    VkQueue presentQueue;
    deviceContext->vkGetDeviceQueue(deviceContext->device, window->swapchain->queueFamilyIndices[0], 0, &presentQueue);

    window->swapchain->setupFramebuffers(cmdBuffer[0]);

    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
    end = start;
    double accumulatedTime = 0.0;
    uint32_t frameCountStart = 0;

    while(true){
        int cmdBufferIndex = (int)frameCount % window->swapchain->imageCount;
        int nextCmdBufferIndex = (cmdBufferIndex == (window->swapchain->imageCount - 1)) ? 0 : (cmdBufferIndex + 1);
        int prevCmdBufferIndex = (cmdBufferIndex == 0) ? (window->swapchain->imageCount - 1) : (cmdBufferIndex - 1);

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
                Projection = glm::perspective(glm::pi<float>() * ((float)VERTICAL_FOV), aspect, 0.1f, 100.f);
            }
        }

        // Update uniforms if duration is greater than zero
        if(start != end){
            std::chrono::duration<double, std::ratio<1, MILLISECONDS_TO_SECONDS>> delta_time = end - start;
            double delta = delta_time.count();
            accumulatedTime += delta;
            if(accumulatedTime > (FRAME_RATE_UPDATE_INTERVAL * MILLISECONDS_TO_SECONDS) ){
                uint32_t deltaFrames = 0;
                if(frameCount < frameCountStart){
                    deltaFrames = ((std::numeric_limits<uint32_t>::max)() - frameCountStart) + frameCount;
                }else{
                    deltaFrames = frameCount - frameCountStart;
                }
                frameCountStart = frameCount;
                double framesPerSecond = (deltaFrames / accumulatedTime) * MILLISECONDS_TO_SECONDS;
                accumulatedTime = 0.0;
                std::cout << "Frames Per Second: " << framesPerSecond << std::endl;
            }
            float angle = (float)(glm::pi<double>() * ((double)ROTATION_RATE) * (delta / MILLISECONDS_TO_SECONDS));

            Model = glm::rotate(Model, angle, glm::vec3(0.0f, 0.4f, 1.0f));
            uniformStruct.MVP = Projection * View * Model;
            uniformStruct.Normal = glm::transpose(glm::inverse(View * Model));
            // matrixBuffer.copyHostData(&uniformStruct, 0, sizeof(uniformStruct));
            start = end;
        }

        // Begin the render pass
        uint32_t numClearValues = (sampleCountFlag != VK_SAMPLE_COUNT_1_BIT) ? 4 : 2;
        std::vector<VkClearValue> clearValues(numClearValues);
        clearValues[0].color = {0.7f, 0.7f, 0.7f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        if(sampleCountFlag != VK_SAMPLE_COUNT_1_BIT){
            clearValues[2].color = {0.7f, 0.7f, 0.7f, 1.0f};
            clearValues[3].depthStencil = {1.0f, 0};
        }

        VkRenderPassBeginInfo renderPassBegin;
        renderPassBegin.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBegin.pNext           = nullptr;
        renderPassBegin.renderPass      = window->swapchain->renderPass;
        renderPassBegin.framebuffer     = window->swapchain->getCurrentFramebuffer();
        renderPassBegin.renderArea      = {{0,0}, window->swapchain->extent};
        renderPassBegin.clearValueCount = clearValues.size();
        renderPassBegin.pClearValues    = &clearValues[0];

        deviceContext->vkQueueWaitIdle(presentQueue);
		// Bind Descriptor Sets
		deviceContext->vkCmdBindDescriptorSets(cmdBuffer[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &samplerDescriptorVector[0], 0, nullptr);
        // Update Push Constants
        deviceContext->vkCmdPushConstants(cmdBuffer[cmdBufferIndex], layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniformStruct), &uniformStruct);
        deviceContext->vkCmdBindPipeline(cmdBuffer[cmdBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, vps.pipeline);
        deviceContext->vkCmdBeginRenderPass(cmdBuffer[cmdBufferIndex], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
        deviceContext->vkCmdBindVertexBuffers(cmdBuffer[cmdBufferIndex], 0, 1, &vertexBuffer.bufferHandle, &vertexOffset);
        deviceContext->vkCmdBindIndexBuffer(cmdBuffer[cmdBufferIndex], indexBuffer.bufferHandle, 0, VK_INDEX_TYPE_UINT16);
        deviceContext->vkCmdDrawIndexed(cmdBuffer[cmdBufferIndex], numIndices, 1, 0, 0, 0);

        // Dispatch
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

        // Present
        try{
            window->swapchain->present(presentQueue);
        }catch(std::runtime_error err){
            std::cout << "Present error: " << err.what() << std::endl;
        }

        // Update timer
        end = std::chrono::high_resolution_clock::now();

        if( frameCount > 0){
            deviceContext->vkResetFences(deviceContext->device, 1, &submitFences[prevCmdBufferIndex]);
            renderPool->resetCommandBuffer(cmdBuffer[prevCmdBufferIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }
        frameCount = (frameCount == (std::numeric_limits<uint32_t>::max)()) ? 0 : (frameCount + 1);
        assert( deviceContext->vkBeginCommandBuffer(cmdBuffer[nextCmdBufferIndex], &cmdBufferBeginInfo) == VK_SUCCESS);

        if(imageWritten == false && frameCount > 60){
            imageWritten = true;
            VkExtent3D imageExtent;
            imageExtent.width = window->swapchain->extent.width;
            imageExtent.height = window->swapchain->extent.height;
            imageExtent.depth = 1;
            VulkanImage frameBufferWrapper(deviceContext, 
                                        window->swapchain->getCurrentImage(),
                                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                        VK_IMAGE_TYPE_2D,
                                        window->swapchain->swapchainFormat,
                                        imageExtent,
                                        0,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        1,
                                        1,
                                        VK_SHARING_MODE_EXCLUSIVE,
                                        0,
                                        nullptr,
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            frameBufferWrapper.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
            frameBufferWrapper.saveImage("FB.png");
        }

        // std::cout << "Frame #" << frameCount << std::endl;

        #if defined (_WIN32) || defined (_WIN64)
            MSG message;

            while( PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)){
                TranslateMessage(&message);
                DispatchMessage(&message);  
            }
        #elif defined (__linux__)
            xcb_generic_event_t *event;

            while((event = xcb_poll_for_event(window->windowInstance))){
                // TODO: Handle more events, resize only for now
                switch(event->response_type & ~0x80){
                    case XCB_CONFIGURE_NOTIFY:
                        {
                            xcb_configure_notify_event_t *configureNotifyEvent = (xcb_configure_notify_event_t*)event;
                            uint32_t configureNotifyWidth = configureNotifyEvent->width;
                            uint32_t configureNotifyHeight = configureNotifyEvent->height;
                            if(configureNotifyWidth != window->swapchain->extent.width || configureNotifyHeight != window->swapchain->extent.height){
                                std::cout << "New Width/Height: " << configureNotifyWidth << "/" << configureNotifyHeight << std::endl;
                                window->swapchain->recreateSwapchain();
                            }
                        }
                        break;
                    default:
                        break;
                }

                delete event;
            }

        #endif
    }

    delete window;
    assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
    renderPool->freeCommandBuffers(window->swapchain->imageCount, &cmdBuffer);
    delete[] cmdBuffer;
    delete renderPool;

    return 0;
}
