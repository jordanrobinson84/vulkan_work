#ifndef __VULKAN_DRIVER_INSTANCE__
#define __VULKAN_DRIVER_INSTANCE__

#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vk_sdk_platform.h>
#include <vector>
#include <iostream>
#include <dlfcn.h> 
#include <cassert>
#include "vulkanCommandPool.h"

#define VK_EXPORTED_FUNCTION(function) PFN_##function function
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) PFN_##function function
#define VK_DEVICE_FUNCTION(function) PFN_##function function

typedef VkQueueFamilyProperties * VkQueueFamilyPropertiesPtr;

struct VulkanDevice{
    VulkanDevice() : created(false){};
    int32_t getUsableMemoryType(uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProperties);
    int32_t getUsableDeviceQueue(const VkQueueFlags requiredProperties);
    VulkanCommandPool * getCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);

    VkDevice device;
    bool created;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkFormatProperties deviceFormatProperties;
    VkImageFormatProperties deviceImageFormatProperties;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkPhysicalDeviceProperties deviceProperties;
    uint32_t deviceQueueFamilyPropertyCount;
    VkQueueFamilyPropertiesPtr deviceQueueProperties;
    VkSparseImageFormatProperties deviceSparseImageFormatProperties;

    // Device-level Function Pointers
    VK_DEVICE_FUNCTION(vkAllocateCommandBuffers);
    VK_DEVICE_FUNCTION(vkAllocateDescriptorSets);
    VK_DEVICE_FUNCTION(vkAllocateMemory);
    VK_DEVICE_FUNCTION(vkBindBufferMemory);
    VK_DEVICE_FUNCTION(vkBindImageMemory);
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
    VK_DEVICE_FUNCTION(vkFreeCommandBuffers);
    VK_DEVICE_FUNCTION(vkFreeDescriptorSets);
    VK_DEVICE_FUNCTION(vkFreeMemory);
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
    VK_DEVICE_FUNCTION(vkResetCommandPool);
    VK_DEVICE_FUNCTION(vkResetDescriptorPool);
    VK_DEVICE_FUNCTION(vkResetEvent);
    VK_DEVICE_FUNCTION(vkResetFences);
    VK_DEVICE_FUNCTION(vkSetEvent);
    VK_DEVICE_FUNCTION(vkUnmapMemory);
    VK_DEVICE_FUNCTION(vkUpdateDescriptorSets);
    VK_DEVICE_FUNCTION(vkWaitForFences);
};


class VulkanDriverInstance{
public:
    VulkanDriverInstance(std::string applicationName);
    ~VulkanDriverInstance();
    void enumeratePhysicalDevices(bool debugPrint = true);
    void setupDevice(uint32_t deviceNumber, bool debugPrint = true);
    VulkanDevice * getDevice(uint32_t deviceNumber, bool debugPrint = true);

    void *loader;
    // Instance variables
    VkInstance instance;
    uint32_t numPhysicalDevices;
    std::vector<VkPhysicalDevice> physicalDevices;

    // Device variables
    std::vector<VulkanDevice> devices;

    // Exported Function Pointers
    VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
    VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr);
    VK_EXPORTED_FUNCTION(vkCreateInstance);
    VK_EXPORTED_FUNCTION(vkDestroyInstance);

    // Instance Function Pointers
    VK_INSTANCE_FUNCTION(vkCreateDevice);
    VK_INSTANCE_FUNCTION(vkDestroyDevice);
    VK_INSTANCE_FUNCTION(vkEnumerateDeviceLayerProperties);
    VK_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceFormatProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceImageFormatProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties);
};

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

#endif