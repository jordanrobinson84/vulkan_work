#ifndef __VULKAN_PIPELINE_STATE__
#define __VULKAN_PIPELINE_STATE__

#include "vulkanDriverInstance.h"

class VulkanPipelineState{
public:
    VulkanPipelineState(VulkanDevice                          * __deviceContext);
    ~VulkanPipelineState();
	void setPrimitiveState(std::vector<VkVertexInputBindingDescription>		&vertexInputBindingDescriptions,
						   std::vector<VkVertexInputAttributeDescription>	&vertexInputAttributeDescriptions,
						   VkPrimitiveTopology								primitiveTopology,
						   VkPolygonMode									polygonMode = VK_POLYGON_MODE_FILL,
						   VkBool32											primitiveRestartEnable = VK_FALSE,
						   float											lineWidth = 1.0f,
						   VkCullModeFlags   								cullMode = VK_CULL_MODE_BACK_BIT,
						   VkFrontFace										frontFaceMode = VK_FRONT_FACE_CLOCKWISE,
						   VkBool32											depthBiasEnable = VK_FALSE,
						   float											depthBiasConstantFactor = 0.0f,
						   float											depthBiasClamp = 0.0f,
						   float											depthBiasSlopeFactor = 0.0f);
	void setViewportState(VkExtent2D &viewportExtent,
						  VkRect2D &scissorRect,
						  std::pair<float, float> viewportOffset = { 0.0f,0.0f },
						  std::pair<float, float> depthRange = { 0.0f, 1.0f });
	bool complete();

    VulkanDevice * deviceContext;
	VkGraphicsPipelineCreateInfo pipelineInfo;
};

#endif