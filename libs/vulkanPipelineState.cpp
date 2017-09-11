#include "vulkanPipelineState.h"

VulkanPipelineState::VulkanPipelineState(VulkanDevice                          * __deviceContext) {
    deviceContext   = __deviceContext;
    isComplete      = false;

    pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.flags                  = 0;
    pipelineInfo.stageCount             = 0;
    pipelineInfo.subpass                = VK_NULL_HANDLE;
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
        pipelineInfo.stageCount = 0;
        pipelineInfo.pStages = nullptr;
    }

    if (pipelineInfo.pViewportState != nullptr) {
        if (pipelineInfo.pViewportState->pViewports != nullptr) {
            delete pipelineInfo.pViewportState->pViewports;
        }

        delete pipelineInfo.pViewportState;
    }

    if (pipelineInfo.pDepthStencilState != nullptr){
        delete pipelineInfo.pDepthStencilState;
    }

    for(auto pair : descriptorSetLayoutBindings){
        auto mapEntry = pair.second;
        mapEntry.clear();
    }
    descriptorSetLayoutBindings.clear();

    for(auto layoutPair : descriptorSetLayouts){
        deviceContext->vkDestroyDescriptorSetLayout(deviceContext->device, layoutPair.second, nullptr);
    }
    descriptorSetLayouts.clear();

    for(auto mapPair : descriptorSets){
        // mapPair is a pair of <VkDescriptorPool, std::vector<VkDescriptorSet>>
        assert(deviceContext->vkFreeDescriptorSets(deviceContext->device, mapPair.first, mapPair.second.size(), &mapPair.second.at(0)) == VK_SUCCESS);
        mapPair.second.clear();
    }
    descriptorSets.clear();

    deviceContext->vkDestroyPipeline(deviceContext->device, pipeline, nullptr);   
}

void VulkanPipelineState::addDescriptorSetLayoutBindings(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings){
    // Get map entry
    std::vector<VkDescriptorSetLayoutBinding> mapEntry;
    try{
        mapEntry = descriptorSetLayoutBindings.at(set);
    }catch(std::out_of_range oor){
        // Add new map entry
        mapEntry = std::vector<VkDescriptorSetLayoutBinding>();
    }
    std::cout << "Adding " << bindings.size() << " bindings" << std::endl;
    mapEntry.insert(mapEntry.begin(), bindings.begin(), bindings.end());
    descriptorSetLayoutBindings.emplace(set, mapEntry);
}

void VulkanPipelineState::addShaderStage(std::string shaderFileName, VkShaderStageFlagBits stage, const std::string entryPointName, VkSpecializationInfo * specialization){
    // Check that the requested stage isn't already used
    VkShaderStageFlags usedFlagTest = unusedStageFlags & ~stage;
    if (usedFlagTest == unusedStageFlags){
        std::cout << "Requested shader stage has already been used:" << std::endl;
        std::cout << "   File name:" << shaderFileName << std::endl;
        std::cout << "   Stage:" << stage << std::endl;
        std::cout << "   Entry point name:" << entryPointName << std::endl;
        return;
    }

    // Get shader code
    std::ifstream shaderStream(shaderFileName, std::ifstream::binary);
    if(shaderStream){
        shaderStream.seekg(0, shaderStream.end);
        int32_t signedCodeSize = (int32_t)shaderStream.tellg();
        assert(signedCodeSize != -1);
        uint32_t codeSize = (uint32_t)signedCodeSize;
        // Seek to beginning
        shaderStream.seekg(0, shaderStream.beg);

        VkShaderModuleCreateInfo shaderInfo;
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.pNext = nullptr;
        shaderInfo.codeSize = (uint32_t)codeSize;

        char * codeBuffer = new char[codeSize];
        shaderStream.read(codeBuffer, codeSize);
        shaderStream.close();
        shaderInfo.pCode = (uint32_t *)(codeBuffer);
        shaderInfo.flags = 0;

        VkShaderModule shaderModule;
        assert(deviceContext->vkCreateShaderModule(deviceContext->device, &shaderInfo, nullptr, &shaderModule) == VK_SUCCESS);

        delete[] codeBuffer;

        // Set up Creation Info
        VkPipelineShaderStageCreateInfo createInfo;
        createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stage                = stage;
        createInfo.module               = shaderModule;
        //createInfo.pName              = entryPointName.c_str();
        createInfo.pName                = "main";
        createInfo.pSpecializationInfo  = specialization;

        shaderStages.push_back(createInfo);

        pipelineInfo.stageCount++;
        unusedStageFlags = usedFlagTest;
    }else{
        std::cout << "Error opening shader file: " << shaderFileName << std::endl;
        assert(shaderStream.is_open());
    }
}

std::vector<VkDescriptorSet>& VulkanPipelineState::generateDescriptorSets(VkDescriptorPool descriptorPool){

    // Check Descriptor Set Layouts
    for(auto pair : descriptorSetLayoutBindings){
        auto mapEntry = pair.second;
        std::map<uint32_t, VkDescriptorSetLayoutBinding> duplicateMap;
        for(VkDescriptorSetLayoutBinding layoutBinding : mapEntry){
            // Check if duplicate
            std::map<uint32_t, VkDescriptorSetLayoutBinding>::iterator existingKeyIterator;
            existingKeyIterator = duplicateMap.find(layoutBinding.binding);
            if(existingKeyIterator != duplicateMap.end()){
                duplicateMap.emplace(layoutBinding.binding, layoutBinding);
                std::cout << "Descriptor Set Binding: ( Set " << pair.first << ", Binding " << layoutBinding.binding << " )" << std::endl;
            }else{
                std::runtime_error("Duplicate layout binding found!");
            }
        }

        if(mapEntry.size() > 0){
            VkDescriptorSetLayoutCreateInfo setLayoutInfo;
            setLayoutInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            setLayoutInfo.pNext         = nullptr;
            setLayoutInfo.flags         = 0;
            setLayoutInfo.bindingCount  = mapEntry.size();
            setLayoutInfo.pBindings     = &mapEntry[0];

            VkDescriptorSetLayout setLayout;
            assert(deviceContext->vkCreateDescriptorSetLayout(deviceContext->device, &setLayoutInfo, nullptr, &setLayout) == VK_SUCCESS);
            descriptorSetLayouts.emplace(pair.first, setLayout);

            // Allocate Descriptor Set
            VkDescriptorSetAllocateInfo descriptorSetInfo;
            descriptorSetInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetInfo.pNext                 = nullptr;
            descriptorSetInfo.descriptorPool        = descriptorPool;
            descriptorSetInfo.descriptorSetCount    = 1;
            descriptorSetInfo.pSetLayouts           = &setLayout;

            VkDescriptorSet descriptorSet;
            assert(deviceContext->vkAllocateDescriptorSets(deviceContext->device, &descriptorSetInfo, &descriptorSet) == VK_SUCCESS);
            // Add map entry if it doesn't exist
            try{
                std::vector<VkDescriptorSet>& entry = descriptorSets.at(descriptorPool);
            }catch(std::out_of_range oor){
                descriptorSets.emplace(descriptorPool, std::vector<VkDescriptorSet>());
            }
            std::cout << "Adding Descriptor Set" << std::endl;
            descriptorSets.at(descriptorPool).push_back(descriptorSet);
        }
    }

    return descriptorSets.at(descriptorPool);
}

void VulkanPipelineState::setMultisampleState(VkSampleCountFlagBits sampleCount, double minSampleShading, const VkSampleMask* sampleMask, VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable){
    // Multisample State
    VkPipelineMultisampleStateCreateInfo * multisampleInfo = new VkPipelineMultisampleStateCreateInfo();
    multisampleInfo->sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->pNext                  = nullptr;
    multisampleInfo->flags                  = 0;
    multisampleInfo->rasterizationSamples   = sampleCount;
    multisampleInfo->sampleShadingEnable    = (sampleCount != VK_SAMPLE_COUNT_1_BIT) ? VK_TRUE : VK_FALSE;
    multisampleInfo->minSampleShading       = minSampleShading;
    multisampleInfo->pSampleMask            = sampleMask;
    multisampleInfo->alphaToCoverageEnable  = alphaToCoverageEnable;
    multisampleInfo->alphaToOneEnable       = alphaToOneEnable;

    if (pipelineInfo.pMultisampleState != nullptr){
        delete pipelineInfo.pMultisampleState;
    }
    pipelineInfo.pMultisampleState = multisampleInfo;
}

void VulkanPipelineState::setPrimitiveState(std::vector<VkVertexInputBindingDescription>    &vertexInputBindingDescriptions,
                                            std::vector<VkVertexInputAttributeDescription>  &vertexInputAttributeDescriptions,
                                            VkPrimitiveTopology                             primitiveTopology,
                                            VkPolygonMode                                   polygonMode,
                                            VkBool32                                        primitiveRestartEnable,
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
    rasterizationInfo->lineWidth                = lineWidth;
    rasterizationInfo->polygonMode              = polygonMode;
    rasterizationInfo->rasterizerDiscardEnable  = VK_FALSE;
    
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
    viewport->width     = (float)viewportExtent.width;
    viewport->height    = (float)viewportExtent.height;
    viewport->minDepth  = depthRange.first;
    viewport->maxDepth  = depthRange.second;

    // Scissor Rectangle
    int32_t swapchainWidth      = (int32_t)viewportExtent.width;
    int32_t swapchainHeight     = (int32_t)viewportExtent.width;
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

    // Recreate pipeline
    if(isComplete){
        std::cout << "Recreating pipeline!" << std::endl;
        assert(deviceContext->vkDeviceWaitIdle(deviceContext->device) == VK_SUCCESS);
        deviceContext->vkDestroyPipeline(deviceContext->device, pipeline, nullptr);
        isComplete = false;
        complete();
    }
}

void VulkanPipelineState::complete() {
    if (!isComplete){
        // TODO: Option to use pipeline caching
        // TODO: Check for minimum viable pipeline
        pipelineInfo.pStages = shaderStages.data();
        assert(pipelineInfo.pStages != nullptr); // Vertex shader required

        if(pipelineInfo.pMultisampleState == nullptr){
            setMultisampleState(VK_SAMPLE_COUNT_1_BIT);
        }

        // Depth stencil state
        VkPipelineDepthStencilStateCreateInfo * depthStencilStateInfo = new VkPipelineDepthStencilStateCreateInfo();
        depthStencilStateInfo->sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateInfo->flags                 = 0;
        depthStencilStateInfo->depthTestEnable       = VK_TRUE;
        depthStencilStateInfo->depthWriteEnable      = VK_TRUE;
        depthStencilStateInfo->depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencilStateInfo->depthBoundsTestEnable = VK_FALSE;
        depthStencilStateInfo->minDepthBounds        = 0.0f;
        depthStencilStateInfo->maxDepthBounds        = 1.0f;
        depthStencilStateInfo->stencilTestEnable     = VK_FALSE;
        depthStencilStateInfo->front                 = {};
        depthStencilStateInfo->back                  = {};

        if (pipelineInfo.pDepthStencilState != nullptr){
            delete pipelineInfo.pDepthStencilState;
        }
        pipelineInfo.pDepthStencilState = depthStencilStateInfo;

        VkResult result = deviceContext->vkCreateGraphicsPipelines(deviceContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline);
        switch(result){
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                std::cout << "VulkanPipelineState - Out of host memory!" << std::endl;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                std::cout << "VulkanPipelineState - Out of device memory!" << std::endl;
            default:
                std::cout << "VulkanPipelineState - Pipeline created." << std::endl;
        }
        assert(result == VK_SUCCESS);
        isComplete = true;
    }
}

bool VulkanPipelineState::completed() {
    return isComplete;
}