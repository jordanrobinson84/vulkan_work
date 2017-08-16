#ifndef __VULKAN_DRIVER_INSTANCE__
#define __VULKAN_DRIVER_INSTANCE__

#if defined (_WIN32) || defined (_WIN64)
    #pragma warning( disable : 4710)
    #pragma warning( disable : 4820)
    #pragma warning( disable : 4191)
    #pragma warning( disable : 4061)
    #include <windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
    #define VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    #define PFN_vkCreateSurfaceKHR PFN_vkCreateWin32SurfaceKHR
    #define vkCreateSurfaceKHR vkCreateWin32SurfaceKHR
    #define PFN_vkGetPhysicalDevicePresentationSupportKHR PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
    #define vkGetPhysicalDevicePresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR
#elif defined (__linux__)
    #include <X11/keysym.h>
    #include <xcb/xcb.h>
    #include <xcb/xcb_keysyms.h>
    #include <xcb/xcb_icccm.h>
    #include <xcb/randr.h>
    #include <dlfcn.h> 
    #include <malloc.h>
    #define VK_USE_PLATFORM_XCB_KHR
    #define VK_KHR_PLATFORM_SURFACE_EXTENSION_NAME  VK_KHR_XCB_SURFACE_EXTENSION_NAME
    #define PFN_vkCreateSurfaceKHR PFN_vkCreateXcbSurfaceKHR
    #define vkCreateSurfaceKHR vkCreateXcbSurfaceKHR
    #define PFN_vkGetPhysicalDevicePresentationSupportKHR PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
    #define vkGetPhysicalDevicePresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR
#endif

#include <vulkan/vk_platform.h>
#include <vulkan/vk_sdk_platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>
#include <memory>
#include <fstream>
#include "vulkanCommandPool.h"

#define VK_EXPORTED_FUNCTION(function) PFN_##function function
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) PFN_##function function
#define VK_DEVICE_FUNCTION(function) PFN_##function function

// Fix stringification of macros
#define macrostr(a) str(a)
#define str(a) #a

typedef VkQueueFamilyProperties * VkQueueFamilyPropertiesPtr;

class VulkanCommandPool;
class VulkanDriverInstance;

struct VulkanDevice{
    VulkanDevice(VulkanDriverInstance * __instance, uint32_t __deviceNumber, bool debugPrint = true);
    ~VulkanDevice();
    VkDescriptorPool *                  getDescriptorPool(const std::vector<VkDescriptorPoolSize>& descriptorSizes);
    uint32_t                            getUsableMemoryType(uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProperties);
    uint32_t                            getUsableDeviceQueueFamily(const VkQueueFlags requiredProperties);
    VulkanCommandPool *                 getCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    //void                                allocateAndBindMemory(VkBuffer buffer, bool hostVisible);
    VkDeviceMemory                      allocateAndBindMemory(VkImage image, bool hostVisible);
    VkFormat                            getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkDevice                            device;
    VulkanDriverInstance *              instance;
    std::vector<VkDescriptorPool>       descriptorPools;
    VkPhysicalDeviceFeatures            deviceFeatures;
    VkFormatProperties                  deviceFormatProperties;
    VkImageFormatProperties             deviceImageFormatProperties;
    VkPhysicalDeviceMemoryProperties    deviceMemoryProperties;
    uint32_t                            deviceNumber;
    VkPhysicalDeviceProperties          deviceProperties;
    uint32_t                            deviceQueueFamilyPropertyCount;
    VkQueueFamilyPropertiesPtr          deviceQueueProperties;
    VkSparseImageFormatProperties       deviceSparseImageFormatProperties;

    // Device-level Function Pointers
    VK_DEVICE_FUNCTION(vkAllocateCommandBuffers);
    VK_DEVICE_FUNCTION(vkAllocateDescriptorSets);
    VK_DEVICE_FUNCTION(vkAllocateMemory);
    VK_DEVICE_FUNCTION(vkBindBufferMemory);
    VK_DEVICE_FUNCTION(vkBindImageMemory);

    // Command Buffers
    VK_DEVICE_FUNCTION(vkBeginCommandBuffer);
    VK_DEVICE_FUNCTION(vkCmdBeginQuery);
    VK_DEVICE_FUNCTION(vkCmdBeginRenderPass);
    VK_DEVICE_FUNCTION(vkCmdBindDescriptorSets);
    VK_DEVICE_FUNCTION(vkCmdBindIndexBuffer);
    VK_DEVICE_FUNCTION(vkCmdBindPipeline);
    VK_DEVICE_FUNCTION(vkCmdBindVertexBuffers);
    VK_DEVICE_FUNCTION(vkCmdBlitImage);
    VK_DEVICE_FUNCTION(vkCmdClearAttachments);
    VK_DEVICE_FUNCTION(vkCmdClearColorImage);
    VK_DEVICE_FUNCTION(vkCmdClearDepthStencilImage);
    VK_DEVICE_FUNCTION(vkCmdCopyBuffer);
    VK_DEVICE_FUNCTION(vkCmdCopyBufferToImage);
    VK_DEVICE_FUNCTION(vkCmdCopyImage);
    VK_DEVICE_FUNCTION(vkCmdCopyImageToBuffer);
    VK_DEVICE_FUNCTION(vkCmdCopyQueryPoolResults);
    VK_DEVICE_FUNCTION(vkCmdDispatch);
    VK_DEVICE_FUNCTION(vkCmdDispatchIndirect);
    VK_DEVICE_FUNCTION(vkCmdDraw);
    VK_DEVICE_FUNCTION(vkCmdDrawIndexed);
    VK_DEVICE_FUNCTION(vkCmdDrawIndexedIndirect);
    VK_DEVICE_FUNCTION(vkCmdDrawIndirect);
    VK_DEVICE_FUNCTION(vkCmdEndQuery);
    VK_DEVICE_FUNCTION(vkCmdEndRenderPass);
    VK_DEVICE_FUNCTION(vkCmdExecuteCommands);
    VK_DEVICE_FUNCTION(vkCmdFillBuffer);
    VK_DEVICE_FUNCTION(vkCmdNextSubpass);
    VK_DEVICE_FUNCTION(vkCmdPipelineBarrier);
    VK_DEVICE_FUNCTION(vkCmdPushConstants);
    VK_DEVICE_FUNCTION(vkCmdResetEvent);
    VK_DEVICE_FUNCTION(vkCmdResetQueryPool);
    VK_DEVICE_FUNCTION(vkCmdResolveImage);
    VK_DEVICE_FUNCTION(vkCmdSetBlendConstants);
    VK_DEVICE_FUNCTION(vkCmdSetDepthBias);
    VK_DEVICE_FUNCTION(vkCmdSetDepthBounds);
    VK_DEVICE_FUNCTION(vkCmdSetEvent);
    VK_DEVICE_FUNCTION(vkCmdSetLineWidth);
    VK_DEVICE_FUNCTION(vkCmdSetScissor);
    VK_DEVICE_FUNCTION(vkCmdSetStencilCompareMask);
    VK_DEVICE_FUNCTION(vkCmdSetStencilReference);
    VK_DEVICE_FUNCTION(vkCmdSetStencilWriteMask);
    VK_DEVICE_FUNCTION(vkCmdSetViewport);
    VK_DEVICE_FUNCTION(vkCmdUpdateBuffer);
    VK_DEVICE_FUNCTION(vkCmdWaitEvents);
    VK_DEVICE_FUNCTION(vkCmdWriteTimestamp);
    VK_DEVICE_FUNCTION(vkEndCommandBuffer);
    VK_DEVICE_FUNCTION(vkQueueSubmit);

    // Creation
    VK_DEVICE_FUNCTION(vkCreateBuffer);
    VK_DEVICE_FUNCTION(vkCreateBufferView);
    VK_DEVICE_FUNCTION(vkCreateCommandPool);
    VK_DEVICE_FUNCTION(vkCreateComputePipelines);
    VK_DEVICE_FUNCTION(vkCreateDescriptorPool);
    VK_DEVICE_FUNCTION(vkCreateDescriptorSetLayout);
    VK_DEVICE_FUNCTION(vkCreateEvent);
    VK_DEVICE_FUNCTION(vkCreateFence);
    VK_DEVICE_FUNCTION(vkCreateFramebuffer);
    VK_DEVICE_FUNCTION(vkCreateGraphicsPipelines);
    VK_DEVICE_FUNCTION(vkCreateImage);
    VK_DEVICE_FUNCTION(vkCreateImageView);
    VK_DEVICE_FUNCTION(vkCreatePipelineCache);
    VK_DEVICE_FUNCTION(vkCreatePipelineLayout);
    VK_DEVICE_FUNCTION(vkCreateQueryPool);
    VK_DEVICE_FUNCTION(vkCreateRenderPass);
    VK_DEVICE_FUNCTION(vkCreateSampler);
    VK_DEVICE_FUNCTION(vkCreateSemaphore);
    VK_DEVICE_FUNCTION(vkCreateShaderModule);

    // Destruction
    VK_DEVICE_FUNCTION(vkDestroyBuffer);
    VK_DEVICE_FUNCTION(vkDestroyBufferView);
    VK_DEVICE_FUNCTION(vkDestroyCommandPool);
    VK_DEVICE_FUNCTION(vkDestroyDescriptorPool);
    VK_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout);
    VK_DEVICE_FUNCTION(vkDestroyEvent);
    VK_DEVICE_FUNCTION(vkDestroyFence);
    VK_DEVICE_FUNCTION(vkDestroyFramebuffer);
    VK_DEVICE_FUNCTION(vkDestroyImage);
    VK_DEVICE_FUNCTION(vkDestroyImageView);
    VK_DEVICE_FUNCTION(vkDestroyPipeline);
    VK_DEVICE_FUNCTION(vkDestroyPipelineCache);
    VK_DEVICE_FUNCTION(vkDestroyPipelineLayout);
    VK_DEVICE_FUNCTION(vkDestroyQueryPool);
    VK_DEVICE_FUNCTION(vkDestroyRenderPass);
    VK_DEVICE_FUNCTION(vkDestroySampler);
    VK_DEVICE_FUNCTION(vkDestroySemaphore);
    VK_DEVICE_FUNCTION(vkDestroyShaderModule);

    VK_DEVICE_FUNCTION(vkDeviceWaitIdle);
    VK_DEVICE_FUNCTION(vkFlushMappedMemoryRanges);

    // Free
    VK_DEVICE_FUNCTION(vkFreeCommandBuffers);
    VK_DEVICE_FUNCTION(vkFreeDescriptorSets);
    VK_DEVICE_FUNCTION(vkFreeMemory);

    // Getters
    VK_DEVICE_FUNCTION(vkGetBufferMemoryRequirements);
    VK_DEVICE_FUNCTION(vkGetDeviceMemoryCommitment);
    VK_DEVICE_FUNCTION(vkGetDeviceQueue);
    VK_DEVICE_FUNCTION(vkGetEventStatus);
    VK_DEVICE_FUNCTION(vkGetFenceStatus);
    VK_DEVICE_FUNCTION(vkGetImageMemoryRequirements);
    VK_DEVICE_FUNCTION(vkGetImageSparseMemoryRequirements);
    VK_DEVICE_FUNCTION(vkGetImageSubresourceLayout);
    VK_DEVICE_FUNCTION(vkGetPipelineCacheData);
    VK_DEVICE_FUNCTION(vkGetQueryPoolResults);
    VK_DEVICE_FUNCTION(vkGetRenderAreaGranularity);

    VK_DEVICE_FUNCTION(vkInvalidateMappedMemoryRanges);
    VK_DEVICE_FUNCTION(vkMapMemory);
    VK_DEVICE_FUNCTION(vkMergePipelineCaches);

	VK_DEVICE_FUNCTION(vkQueueWaitIdle);

    // Reset
    VK_DEVICE_FUNCTION(vkResetCommandBuffer);
    VK_DEVICE_FUNCTION(vkResetCommandPool);
    VK_DEVICE_FUNCTION(vkResetDescriptorPool);
    VK_DEVICE_FUNCTION(vkResetEvent);
    VK_DEVICE_FUNCTION(vkResetFences);

    VK_DEVICE_FUNCTION(vkSetEvent);
    VK_DEVICE_FUNCTION(vkUnmapMemory);
    VK_DEVICE_FUNCTION(vkUpdateDescriptorSets);
    VK_DEVICE_FUNCTION(vkWaitForFences);

    // Extensions
    VK_DEVICE_FUNCTION(vkCreateSwapchainKHR);
    VK_DEVICE_FUNCTION(vkDestroySwapchainKHR);
    VK_DEVICE_FUNCTION(vkGetSwapchainImagesKHR);
    VK_DEVICE_FUNCTION(vkAcquireNextImageKHR);
    VK_DEVICE_FUNCTION(vkQueuePresentKHR);
};


class VulkanDriverInstance{
public:
    VulkanDriverInstance(std::string applicationName);
    ~VulkanDriverInstance();
    void enumeratePhysicalDevices(bool debugPrint = true);

#if defined (_WIN32) || defined (_WIN64)
	HMODULE loader;
#else
    void *loader;
#endif
    // Instance variables
    VkInstance                      instance;
    uint32_t                        numPhysicalDevices;
    std::vector<VkPhysicalDevice>   physicalDevices;

    // Exported Function Pointers
    VK_EXPORTED_FUNCTION(vkEnumerateInstanceLayerProperties);
    VK_EXPORTED_FUNCTION(vkEnumerateInstanceExtensionProperties);
    VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
    VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr);
    VK_EXPORTED_FUNCTION(vkCreateInstance);
    VK_EXPORTED_FUNCTION(vkDestroyInstance);

    // Instance Function Pointers
    VK_INSTANCE_FUNCTION(vkCreateDevice);
    VK_INSTANCE_FUNCTION(vkDestroyDevice);
    VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices);
    VK_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceFormatProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceImageFormatProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties);

    // Window System Integration
    VK_INSTANCE_FUNCTION(vkCreateSurfaceKHR);
    VK_INSTANCE_FUNCTION(vkDestroySurfaceKHR);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDevicePresentationSupportKHR);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
};

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

#endif