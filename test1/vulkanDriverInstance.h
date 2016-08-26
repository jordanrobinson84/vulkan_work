#ifndef __VULKAN_DRIVER_INSTANCE__
#define __VULKAN_DRIVER_INSTANCE__

#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include <iostream>
#include <dlfcn.h> 

// #include "vulkanDevice.h"

#define VK_EXPORTED_FUNCTION(function) PFN_##function function
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) PFN_##function function
#define VK_DEVICE_FUNCTION(function) PFN_##function function

typedef VkQueueFamilyProperties * VkQueueFamilyPropertiesPtr;

struct VulkanDevice{
    // VulkanDevice();

    VkDevice *device;

    VK_DEVICE_FUNCTION(vkDeviceWaitIdle);
    VK_DEVICE_FUNCTION(vkEnumerateDeviceLayerProperties);
    VK_DEVICE_FUNCTION(vkCreateImage);
};


class VulkanDriverInstance{
public:
    VulkanDriverInstance(std::string applicationName);
    void enumeratePhysicalDevices();

    void *loader;
    // Instance variables
    VkInstance instance;
    uint32_t numPhysicalDevices;
    VkPhysicalDevice *physicalDevices;
    VkQueueFamilyPropertiesPtr *physicalDeviceQueueProperties;

    // Device variables
    VulkanDevice *devices;

    VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
    VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr);
    VK_EXPORTED_FUNCTION(vkCreateInstance);
    VK_EXPORTED_FUNCTION(vkDestroyInstance);
    VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties);
    VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_INSTANCE_FUNCTION(vkEnumerateInstanceLayerProperties);
    VK_INSTANCE_FUNCTION(vkCreateDevice);
    VK_INSTANCE_FUNCTION(vkDestroyDevice);
};

#undef VK_EXPORTED_FUNCTION
#undef VK_GLOBAL_FUNCTION
#undef VK_INSTANCE_FUNCTION
#undef VK_DEVICE_FUNCTION

#endif