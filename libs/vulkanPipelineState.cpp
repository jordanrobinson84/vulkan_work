#include "vulkanPipelineState.h"

VulkanPipelineState::VulkanPipelineState(VulkanDevice                          * __deviceContext) {
	deviceContext = __deviceContext;
	pipelineInfo.pColorBlendState		= nullptr;
	pipelineInfo.pDepthStencilState		= nullptr;
	pipelineInfo.pDynamicState			= nullptr;
	pipelineInfo.pInputAssemblyState	= nullptr;
	pipelineInfo.pMultisampleState		= nullptr;
	pipelineInfo.pNext					= nullptr;
	pipelineInfo.pRasterizationState	= nullptr;
	pipelineInfo.pStages				= nullptr;
	pipelineInfo.pTessellationState		= nullptr;
	pipelineInfo.pVertexInputState		= nullptr;
	pipelineInfo.pViewportState			= nullptr;
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
}
void VulkanPipelineState::setPrimitiveState(std::vector<VkVertexInputBindingDescription>	&vertexInputBindingDescriptions,
											std::vector<VkVertexInputAttributeDescription>	&vertexInputAttributeDescriptions,
											VkPrimitiveTopology								primitiveTopology,
											VkPolygonMode									polygonMode,
											VkBool32										primitiveRestartEnable ,
											float											lineWidth,
											VkCullModeFlags   								cullMode,
											VkFrontFace										frontFaceMode,
											VkBool32										depthBiasEnable,
											float											depthBiasConstantFactor,
											float											depthBiasClamp,
											float											depthBiasSlopeFactor) {
	// Vertex Input
	VkPipelineVertexInputStateCreateInfo * vertexInputInfo = new VkPipelineVertexInputStateCreateInfo();
	vertexInputInfo->sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo->pNext								= nullptr;
	vertexInputInfo->vertexAttributeDescriptionCount	= vertexInputAttributeDescriptions.size();
	vertexInputInfo->pVertexAttributeDescriptions		= vertexInputAttributeDescriptions.size() > 0 ? &vertexInputAttributeDescriptions[0] : nullptr;
	vertexInputInfo->vertexBindingDescriptionCount		= vertexInputBindingDescriptions.size();
	vertexInputInfo->pVertexBindingDescriptions			= vertexInputBindingDescriptions.size() > 0 ? &vertexInputBindingDescriptions[0] : nullptr;

	if (pipelineInfo.pVertexInputState != nullptr) {
		delete pipelineInfo.pVertexInputState;
	}
	pipelineInfo.pVertexInputState = vertexInputInfo;
	
	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo * inputAssemblyInfo = new VkPipelineInputAssemblyStateCreateInfo();
	inputAssemblyInfo->sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo->pNext						= nullptr;
	inputAssemblyInfo->primitiveRestartEnable		= primitiveRestartEnable;
	inputAssemblyInfo->topology						= primitiveTopology;

	if (pipelineInfo.pInputAssemblyState != nullptr) {
		delete pipelineInfo.pInputAssemblyState;
	}
	pipelineInfo.pInputAssemblyState = inputAssemblyInfo;

	//Rasterizer
	VkPipelineRasterizationStateCreateInfo * rasterizationInfo = new VkPipelineRasterizationStateCreateInfo();
	rasterizationInfo->sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo->pNext					= nullptr;
	rasterizationInfo->cullMode					= cullMode;
	rasterizationInfo->frontFace				= frontFaceMode;
	rasterizationInfo->depthBiasEnable			= depthBiasEnable;
	rasterizationInfo->depthBiasConstantFactor	= depthBiasConstantFactor;
	rasterizationInfo->depthBiasClamp			= depthBiasClamp;
	rasterizationInfo->depthBiasSlopeFactor		= depthBiasSlopeFactor;
	
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
	viewport->x			= viewportOffset.first;
	viewport->y			= viewportOffset.second;
	viewport->width		= viewportExtent.width;
	viewport->height	= viewportExtent.height;
	viewport->minDepth	= 0.0f;
	viewport->maxDepth	= 1.0f;

	// Scissor Rectangle
	int32_t swapchainWidth		= viewportExtent.width;
	int32_t swapchainHeight		= viewportExtent.width;
	scissorRect.offset.x		= (std::max)(scissorRect.offset.x, (std::min)(scissorRect.offset.x, swapchainWidth));
	scissorRect.offset.y		= (std::max)(scissorRect.offset.y, (std::min)(scissorRect.offset.y, swapchainHeight));
	uint32_t extentBoundsX		= viewportExtent.width - scissorRect.offset.x;
	uint32_t extentBoundsY		= viewportExtent.height - scissorRect.offset.y;
	scissorRect.extent.width	= (std::max)(extentBoundsX, (std::min)(scissorRect.extent.width, viewportExtent.width));
	scissorRect.extent.height	= (std::max)(extentBoundsY, (std::min)(scissorRect.extent.height, viewportExtent.height));
	
	// Viewport State Info
	VkPipelineViewportStateCreateInfo * viewportInfo = new VkPipelineViewportStateCreateInfo();
	viewportInfo->sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo->pNext			= nullptr;
	viewportInfo->viewportCount = viewport == nullptr ? 0 : 1;
	viewportInfo->pViewports	= viewport;
	viewportInfo->scissorCount	= 1;
	viewportInfo->pScissors		= &scissorRect;

	if (pipelineInfo.pViewportState != nullptr) {
		if (pipelineInfo.pViewportState->pViewports != nullptr) {
			delete pipelineInfo.pViewportState->pViewports;
		}

		delete pipelineInfo.pViewportState;
	}

	pipelineInfo.pViewportState = viewportInfo;
}
bool VulkanPipelineState::complete() {
	return true;
}