#ifndef __VULKAN_PIPELINE_SHADER_STAGE_H__
#define __VULKAN_PIPELINE_SHADER_STAGE_H__

#include "vulkanDriverInstance.h"

class VulkanPipelineShaderStage{
public:
    VulkanPipelineShaderStage(VulkanDevice * deviceContext, VkShaderStageFlagBits stage, VkShaderModule module, const char * pName, bool deferCreation);

    void addSpecializationInfo(uint32_t constantID, uint32_t constantOffset, size_t constantSize, const void * pData);

    // Do not add more data members to ensure equivalence to the below struct
    VkPipelineShaderStageCreateInfo creationInfo;
};
#endif