#include "vulkanPipelineState.h"

VulkanPipelineState::VulkanPipelineState(VulkanDevice                          * __deviceContext) {
    deviceContext   = __deviceContext;
    isComplete      = true;

    pipelineInfo.pColorBlendState       = nullptr;
    pipelineInfo.pDepthStencilState     = nullptr;
    pipelineInfo.pDynamicState          = nullptr;
    pipelineInfo.pInputAssemblyState    = nullptr;
    pipelineInfo.pMultisampleState      = nullptr;
    pipelineInfo.pNext                  = nullptr;
    pipelineInfo.pRasterizationState    = nullptr;
    pipelineInfo.pStages                = nullptr;
    pipelineInfo.pTessellationState     = nullptr;
    pipelineInfo.pVertexInputState      = nullptr;
    pipelineInfo.pViewportState         = nullptr;

    unusedStageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;
}

VulkanPipelineState::~VulkanPipelineState() {
    // Clean up dynamically allocated objects
    if (pipelineInfo.pVertexInputState != nullptr) {
        delete pipelineInfo.pVertexInputState;
    }

    if (pipelineInfo.pInputAssemblyState != nullptr) {
        delete pipelineInfo.pInputAssemblyState;
    }

    if (pipelineInfo.pRasterizationState != nullptr) {
        delete pipelineInfo.pRasterizationState;
    }

    if (pipelineInfo.pStages != nullptr){
        shaderStages.clear();
        pipelineInfo.pStages = nullptr;
    }

    if (pipelineInfo.pViewportState != nullptr) {
        if (pipelineInfo.pViewportState->pViewports != nullptr) {
            delete pipelineInfo.pViewportState->pViewports;
        }

        delete pipelineInfo.pViewportState;
    }
}

void VulkanPipelineState::addShaderStage(std::string shaderFileName, VkShaderStageFlagBits stage, std::string entryPointName, VkSpecializationInfo * specialization){
    // Check that the requested stage isn't already used
    VkShaderStageFlags usedFlagTest = unusedStageFlags | stage;
    if (usedFlagTest != unusedStageFlags){
        std::cerr << "Requested shader stage has already been used:" << std::endl;
        std::cerr << "   File name:" << shaderFileName << std::endl;
        std::cerr << "   Stage:" << stage << std::endl;
        std::cerr << "   Entry point name:" << entryPointName << std::endl;
        return;
    }

    // Get shader code
    std::ifstream shaderStream(shaderFileName, std::ifstream::binary | std::ifstream::ate);
    if(shaderStream.is_open()){
        uint32_t codeSize = shaderStream.tellg();
        // Seek to beginning
        shaderStream.seekg(0, shaderStream.beg);

        VkShaderModuleCreateInfo shaderInfo;
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.pNext = nullptr;
        shaderInfo.codeSize = codeSize;

        char * codeBuffer = new char[codeSize];
        shaderStream.read(codeBuffer, codeSize);
        shaderStream.close();
        shaderInfo.pCode = (uint32_t *)(&codeBuffer);
        shaderInfo.flags = 0;

        VkShaderModule shaderModule;
        deviceContext->vkCreateShaderModule(deviceContext->device, &shaderInfo, nullptr, &shaderModule);

        delete[] codeBuffer;

        // Set up Creation Info
        VkPipelineShaderStageCreateInfo createInfo;
        createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stage                = stage;
        createInfo.module               = shaderModule;
        createInfo.pName                = entryPointName.c_str();
        createInfo.pSpecializationInfo  = specialization;

        shaderStages.push_back(createInfo);

        if (pipelineInfo.pStages == nullptr){
            pipelineInfo.pStages = &shaderStages[0];
            pipelineInfo.stageCount = 1;
        }
        pipelineInfo.stageCount++;
        unusedStageFlags = usedFlagTest;
    }else{
        std::cerr << "Error opening shader file: " << shaderFileName << std::endl;
    }
}

void VulkanPipelineState::setPrimitiveState(std::vector<VkVertexInputBindingDescription>    &vertexInputBindingDescriptions,
                                            std::vector<VkVertexInputAttributeDescription>  &vertexInputAttributeDescriptions,
                                            VkPrimitiveTopology                             primitiveTopology,
                                            VkPolygonMode                                   polygonMode,
                                            VkBool32                                        primitiveRestartEnable ,
                                            float                                           lineWidth,
                                            VkCullModeFlags                                 cullMode,
                                            VkFrontFace                                     frontFaceMode,
                                            VkBool32                                        depthBiasEnable,
                                            float                                           depthBiasConstantFactor,
                                            float                                           depthBiasClamp,
                                            float                                           depthBiasSlopeFactor) {
    // Vertex Input
    VkPipelineVertexInputStateCreateInfo * vertexInputInfo = new VkPipelineVertexInputStateCreateInfo();
    vertexInputInfo->sType                              = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext                              = nullptr;
    vertexInputInfo->vertexAttributeDescriptionCount    = vertexInputAttributeDescriptions.size();
    vertexInputInfo->pVertexAttributeDescriptions       = vertexInputAttributeDescriptions.size() > 0 ? &vertexInputAttributeDescriptions[0] : nullptr;
    vertexInputInfo->vertexBindingDescriptionCount      = vertexInputBindingDescriptions.size();
    vertexInputInfo->pVertexBindingDescriptions         = vertexInputBindingDescriptions.size() > 0 ? &vertexInputBindingDescriptions[0] : nullptr;

    if (pipelineInfo.pVertexInputState != nullptr) {
        delete pipelineInfo.pVertexInputState;
    }
    pipelineInfo.pVertexInputState = vertexInputInfo;
    
    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo * inputAssemblyInfo = new VkPipelineInputAssemblyStateCreateInfo();
    inputAssemblyInfo->sType                        = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->pNext                        = nullptr;
    inputAssemblyInfo->primitiveRestartEnable       = primitiveRestartEnable;
    inputAssemblyInfo->topology                     = primitiveTopology;

    if (pipelineInfo.pInputAssemblyState != nullptr) {
        delete pipelineInfo.pInputAssemblyState;
    }
    pipelineInfo.pInputAssemblyState = inputAssemblyInfo;

    //Rasterizer
    VkPipelineRasterizationStateCreateInfo * rasterizationInfo = new VkPipelineRasterizationStateCreateInfo();
    rasterizationInfo->sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo->pNext                    = nullptr;
    rasterizationInfo->cullMode                 = cullMode;
    rasterizationInfo->frontFace                = frontFaceMode;
    rasterizationInfo->depthBiasEnable          = depthBiasEnable;
    rasterizationInfo->depthBiasConstantFactor  = depthBiasConstantFactor;
    rasterizationInfo->depthBiasClamp           = depthBiasClamp;
    rasterizationInfo->depthBiasSlopeFactor     = depthBiasSlopeFactor;
    
    if (pipelineInfo.pRasterizationState != nullptr) {
        delete pipelineInfo.pRasterizationState;
    }
    pipelineInfo.pRasterizationState = rasterizationInfo;

}

void VulkanPipelineState::setViewportState(VkExtent2D &viewportExtent,
                                           VkRect2D &scissorRect,
                                           std::pair<float, float> viewportOffset,
                                           std::pair<float, float> depthRange) {
    assert(viewportExtent.width > 0 && viewportExtent.width != (std::numeric_limits<uint32_t>::max)());
    assert(viewportExtent.height > 0 && viewportExtent.height != (std::numeric_limits<uint32_t>::max)());

    // Viewport
    VkViewport * viewport = new VkViewport();
    viewport->x         = viewportOffset.first;
    viewport->y         = viewportOffset.second;
    viewport->width     = viewportExtent.width;
    viewport->height    = viewportExtent.height;
    viewport->minDepth  = 0.0f;
    viewport->maxDepth  = 1.0f;

    // Scissor Rectangle
    int32_t swapchainWidth      = viewportExtent.width;
    int32_t swapchainHeight     = viewportExtent.width;
    scissorRect.offset.x        = (std::max)(scissorRect.offset.x, (std::min)(scissorRect.offset.x, swapchainWidth));
    scissorRect.offset.y        = (std::max)(scissorRect.offset.y, (std::min)(scissorRect.offset.y, swapchainHeight));
    uint32_t extentBoundsX      = viewportExtent.width - scissorRect.offset.x;
    uint32_t extentBoundsY      = viewportExtent.height - scissorRect.offset.y;
    scissorRect.extent.width    = (std::max)(extentBoundsX, (std::min)(scissorRect.extent.width, viewportExtent.width));
    scissorRect.extent.height   = (std::max)(extentBoundsY, (std::min)(scissorRect.extent.height, viewportExtent.height));
    
    // Viewport State Info
    VkPipelineViewportStateCreateInfo * viewportInfo = new VkPipelineViewportStateCreateInfo();
    viewportInfo->sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo->pNext         = nullptr;
    viewportInfo->viewportCount = viewport == nullptr ? 0 : 1;
    viewportInfo->pViewports    = viewport;
    viewportInfo->scissorCount  = 1;
    viewportInfo->pScissors     = &scissorRect;

    if (pipelineInfo.pViewportState != nullptr) {
        if (pipelineInfo.pViewportState->pViewports != nullptr) {
            delete pipelineInfo.pViewportState->pViewports;
        }

        delete pipelineInfo.pViewportState;
    }

    pipelineInfo.pViewportState = viewportInfo;
}

void VulkanPipelineState::complete() {
    if (!isComplete){
        // TODO: Option to use pipeline caching
        assert(deviceContext->vkCreateGraphicsPipelines(deviceContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline) == VK_SUCCESS);
        isComplete = true;
    }
}

bool VulkanPipelineState::completed() {
    return isComplete;
}