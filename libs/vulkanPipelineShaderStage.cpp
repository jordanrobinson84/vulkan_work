#include "vulkanPipelineShaderStage.h"

VulkanPipelineShaderStage::VulkanPipelineShaderStage(VkShaderStageFlagBits stage, VkShaderModule module, const char * pName){

    // Set up Creation Info
    createInfo.sType                = VK_STRUCTURE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.stage                = stage;
    createInfo.module               = module;
    createInfo.pName                = pName;
    createInfo.pSpecializationInfo  = nullptr;
}