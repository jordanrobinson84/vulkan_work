#ifndef __VULKAN_PIPELINE_STATE__
#define __VULKAN_PIPELINE_STATE__

#include "vulkanDriverInstance.h"

typedef std::map< VkDescriptorPool, std::vector< VkDescriptorSet> > DescriptorSetMap;
typedef std::map< uint32_t, std::vector< VkDescriptorSetLayoutBinding> > DescriptorSetLayoutBindingMap;

class VulkanPipelineState{
public:
    VulkanPipelineState(VulkanDevice                          * __deviceContext);

    ~VulkanPipelineState();

    void addDescriptorSetLayoutBindings(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    void addShaderStage(std::string shaderFileName, VkShaderStageFlagBits stage, const std::string entryPointName, VkSpecializationInfo * specialization = nullptr);
    std::vector<VkDescriptorSet>& generateDescriptorSets(VkDescriptorPool descriptorPool);
    void setMultisampleState(VkSampleCountFlagBits sampleCount, double minSampleShading = 1.0, const VkSampleMask* sampleMask = nullptr, VkBool32 alphaToCoverageEnable = VK_FALSE, VkBool32 alphaToOneEnable = VK_FALSE);
    void setPrimitiveState(std::vector<VkVertexInputBindingDescription>     &vertexInputBindingDescriptions,
                           std::vector<VkVertexInputAttributeDescription>   &vertexInputAttributeDescriptions,
                           VkPrimitiveTopology                              primitiveTopology,
                           VkPolygonMode                                    polygonMode = VK_POLYGON_MODE_FILL,
                           VkBool32                                         primitiveRestartEnable = VK_FALSE,
                           float                                            lineWidth = 1.0f,
                           VkCullModeFlags                                  cullMode = VK_CULL_MODE_BACK_BIT,
                           VkFrontFace                                      frontFaceMode = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                           VkBool32                                         depthBiasEnable = VK_FALSE,
                           float                                            depthBiasConstantFactor = 0.0f,
                           float                                            depthBiasClamp = 0.0f,
                           float                                            depthBiasSlopeFactor = 0.0f);

    void setViewportState(VkExtent2D &viewportExtent,
                          VkRect2D &scissorRect,
                          std::pair<float, float> viewportOffset = { 0.0f,0.0f },
                          std::pair<float, float> depthRange = { 0.0f, 1.0f });

    void updatePipeline();
    void complete();
    bool completed();

    DescriptorSetMap                                descriptorSets;
    DescriptorSetLayoutBindingMap                   descriptorSetLayoutBindings;
    std::map<uint32_t, VkDescriptorSetLayout>       descriptorSetLayouts;
    VulkanDevice *                                  deviceContext;
    bool                                            isComplete;
    VkGraphicsPipelineCreateInfo                    pipelineInfo;
    VkPipeline                                      pipeline;
    std::vector<VkPipelineShaderStageCreateInfo>    shaderStages;
    VkShaderStageFlags                              unusedStageFlags;
};

#endif