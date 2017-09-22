#ifndef __VULKAN_PIPELINE_STATE__
#define __VULKAN_PIPELINE_STATE__

#include "VulkanDriverInstance.h"

typedef std::map< VkDescriptorPool, std::vector< VkDescriptorSet> > DescriptorSetMap;
typedef std::map< uint32_t, std::vector< VkDescriptorSetLayoutBinding> > DescriptorSetLayoutBindingMap;

class VulkanPipelineState{
public:
    VulkanPipelineState(VulkanDevice                          * __deviceContext);

    ~VulkanPipelineState();

    void addDescriptorSetLayoutBindings(uint32_t set, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    void addShaderStage(std::string shaderFileName, VkShaderStageFlagBits stage, const std::string entryPointName, VkSpecializationInfo * specialization = nullptr);
    std::string descriptorTypeToString(VkDescriptorType type){
      switch(type){
        case VK_DESCRIPTOR_TYPE_SAMPLER:
          return "VK_DESCRIPTOR_TYPE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
          return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
          return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
          return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
          return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
          return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
          return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
          return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
          return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
          return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
          return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
        default:
          return "VK_DESCRIPTOR_TYPE_UNKNOWN";
      }
    };
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