#include <cassert>
#include "vulkanDriverInstance.h"

#if defined (_WIN32) || defined (_WIN64)
    #define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)GetProcAddress(loader, #function ); assert( function != nullptr);
#elif defined(__linux__)
    #define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function ); assert( function != nullptr);
#endif
#define VK_GLOBAL_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( nullptr, #function )); assert( function != nullptr);
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, macrostr(function) )); assert( function != nullptr);
#define VK_DEVICE_FUNCTION(function) function = (PFN_##function)( instance->vkGetDeviceProcAddr( device, #function )); assert( function != nullptr);

VulkanDevice::VulkanDevice(VulkanDriverInstance * __instance, uint32_t deviceNumber, bool debugPrint){
    instance = __instance;
    assert(instance != nullptr);
    assert (deviceNumber >= 0 && deviceNumber < instance->numPhysicalDevices);

    // Get Properties
    instance->vkGetPhysicalDeviceProperties(instance->physicalDevices[deviceNumber], &deviceProperties);

    if (debugPrint){
        uint32_t majorVersion = VK_VERSION_MAJOR(deviceProperties.apiVersion);
        uint32_t minorVersion = VK_VERSION_MINOR(deviceProperties.apiVersion);
        uint32_t patchVersion = VK_VERSION_PATCH(deviceProperties.apiVersion);

        std::cout << "   Physical Device " << std::hex << deviceNumber << ": " << std::endl;
        std::cout << "      API Version: " << std::dec << majorVersion << "." << minorVersion << "." << patchVersion << std::endl;
        std::cout << "      Driver Version: " << std::hex << deviceProperties.driverVersion << std::endl;
        std::cout << "      Vendor ID: " << std::hex << deviceProperties.vendorID << std::endl;
        std::cout << "      Device ID: " << std::hex << deviceProperties.deviceID << std::endl;

        std::string deviceTypeString = ""; 
        switch(deviceProperties.deviceType){
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                deviceTypeString = "Other";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                deviceTypeString = "Integrated GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                deviceTypeString = "Discrete GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                deviceTypeString = "Virtual GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                deviceTypeString = "CPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE:
                deviceTypeString = "Range Size";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
                deviceTypeString = "MAX";
                break;
        }

        std::cout << "      Device Type: " << deviceTypeString << std::endl;
        std::cout << "      Device Name: " << deviceProperties.deviceName << std::endl;
    }

    // Enumerate Physical Device Queue Family Properties
    deviceQueueFamilyPropertyCount = 0;
    instance->vkGetPhysicalDeviceQueueFamilyProperties(instance->physicalDevices[deviceNumber], &deviceQueueFamilyPropertyCount, nullptr);
    assert( deviceQueueFamilyPropertyCount != 0);

    if (debugPrint){
        std::cout << "      Found " << deviceQueueFamilyPropertyCount << " Queue Families: " << std::endl;
    }
    deviceQueueProperties = new VkQueueFamilyProperties[deviceQueueFamilyPropertyCount];
    instance->vkGetPhysicalDeviceQueueFamilyProperties(instance->physicalDevices[deviceNumber], &deviceQueueFamilyPropertyCount, deviceQueueProperties);

    for (uint32_t queueFamily = 0; queueFamily < deviceQueueFamilyPropertyCount; queueFamily++){
        auto queueFamilyProperties = deviceQueueProperties[queueFamily];

        if (debugPrint){
            std::cout << "          Queue Family " << queueFamily << ": " << std::endl;
            std::string properties  = "";
            uint32_t propertyBits[] = { VK_QUEUE_GRAPHICS_BIT, 
                                        VK_QUEUE_COMPUTE_BIT, 
                                        VK_QUEUE_TRANSFER_BIT, 
                                        VK_QUEUE_SPARSE_BINDING_BIT };
    
            std::string propertyBitStrings[] = {"VK_QUEUE_GRAPHICS_BIT", 
                                                "VK_QUEUE_COMPUTE_BIT", 
                                                "VK_QUEUE_TRANSFER_BIT", 
                                                "VK_QUEUE_SPARSE_BINDING_BIT"};
    
            int propertyIndex = 0;
            bool firstProperty = true;
            for (auto property : propertyBits){
                if ((property & queueFamilyProperties.queueFlags) == 0){
                    properties = firstProperty ? properties : properties + " |\n                 ";
                    firstProperty = false;
                    properties += propertyBitStrings[propertyIndex];
                }
                propertyIndex++;
            }
            std::cout << "              Queue Flags: " << std::hex << properties << std::endl;
            std::cout << "              Queue Count: " << queueFamilyProperties.queueCount << std::endl;
            std::cout << "              Timestamp Valid Bits: " << queueFamilyProperties.timestampValidBits << std::endl;
            VkExtent3D minTransferGranularity = queueFamilyProperties.minImageTransferGranularity;
            std::cout << "              Min Image Transfer Granularity: " << std::dec << "(" << minTransferGranularity.width << minTransferGranularity.height << minTransferGranularity.depth << ")" << std::endl;
        }
    }

    // Get Memory Properties
    instance->vkGetPhysicalDeviceMemoryProperties(instance->physicalDevices[deviceNumber], &deviceMemoryProperties);

    if (debugPrint){
        // Print Memory Properties
        std::cout << "Memory Properties for Device " << deviceNumber << ": " << std::endl;
        std::cout << "   Memory Types: " << std::endl;
        for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < deviceMemoryProperties.memoryTypeCount; memoryTypeIndex++){
            VkMemoryType memoryType = deviceMemoryProperties.memoryTypes[memoryTypeIndex];
            std::string properties = "\n         Heap Index ";
            properties += memoryType.heapIndex;
            properties += "\n         ";
            uint32_t propertyBits[] = { VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                                        VK_MEMORY_PROPERTY_HOST_CACHED_BIT, 
                                        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT};
    
            std::string propertyBitStrings[] = {"VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT", 
                                                "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT", 
                                                "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT", 
                                                "VK_MEMORY_PROPERTY_HOST_CACHED_BIT", 
                                                "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT"};
    
            int propertyIndex = 0;
            bool firstProperty = true;
            for (auto property : propertyBits){
                if ((property & memoryType.propertyFlags) == 0){
                    properties = firstProperty ? properties : properties + " |\n         ";
                    firstProperty = false;
                    properties += propertyBitStrings[propertyIndex];
                }
                propertyIndex++;
            }
    
            std::cout << "      Memory Type " << memoryTypeIndex << ": " << properties << std::endl;
        }
    
        std::cout << "   Memory Heaps: " << std::endl;
        for (uint32_t memoryHeapIndex = 0; memoryHeapIndex < deviceMemoryProperties.memoryHeapCount; memoryHeapIndex++){
            VkMemoryHeap memoryHeap = deviceMemoryProperties.memoryHeaps[memoryHeapIndex];
            std::cout << "      Memory Heap " << memoryHeapIndex << ": " << memoryHeap.size << std::endl;
        }
    }

    // Device Queue Create Info
    std::vector<VkDeviceQueueCreateInfo> queueInfo(deviceQueueFamilyPropertyCount);
    for (uint32_t queueFamily = 0; queueFamily < deviceQueueFamilyPropertyCount; queueFamily++){
        auto queueFamilyProperties = deviceQueueProperties[queueFamily];
        
        queueInfo[queueFamily].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo[queueFamily].pNext = nullptr;
        queueInfo[queueFamily].flags = 0;
        queueInfo[queueFamily].queueFamilyIndex = 0;
        queueInfo[queueFamily].queueCount = queueFamilyProperties.queueCount;
        float * queuePriorities = new float[queueInfo[queueFamily].queueCount];
        for (uint32_t i = 0; i < queueInfo[queueFamily].queueCount; i++){
            queuePriorities[i] = 1.0; // Leave at same priority for now
        }
        queueInfo[queueFamily].pQueuePriorities = queuePriorities;
        queueFamily++;
    }

    // Extensions
    std::vector<const char*> requestedExtensions = {"VK_KHR_swapchain"};
    std::vector<const char*> enabledExtensions;
    uint32_t extensionCount = 0;
    assert(instance->vkEnumerateDeviceExtensionProperties(instance->physicalDevices[deviceNumber], nullptr, &extensionCount, nullptr) == VK_SUCCESS);
    std::cout << "Found " << extensionCount << " extensions." << std::endl;
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    assert(instance->vkEnumerateDeviceExtensionProperties(instance->physicalDevices[deviceNumber], nullptr, &extensionCount, &availableExtensions[0]) == VK_SUCCESS);
    for (auto extensionProperties : availableExtensions){
        std::cout << "Extension Name: " << extensionProperties.extensionName << std::endl;
        std::cout << "   Extension Spec Version: " << extensionProperties.specVersion << std::endl;

        for (auto requestedExtension : requestedExtensions){
            if (strcmp(requestedExtension, extensionProperties.extensionName) == 0){
                enabledExtensions.push_back(requestedExtension);
            }
        }
    }

    // Create Info
    VkDeviceCreateInfo creationInfo;
    creationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    creationInfo.pNext = nullptr;
    creationInfo.flags = 0; // Reserved
    creationInfo.queueCreateInfoCount = 1;
    creationInfo.pQueueCreateInfos = &queueInfo[0];
    creationInfo.enabledLayerCount = 0;
    creationInfo.ppEnabledLayerNames = nullptr;
    creationInfo.enabledExtensionCount = 1;
    creationInfo.ppEnabledExtensionNames = &enabledExtensions[0];
    creationInfo.pEnabledFeatures = nullptr;

    // Create Device
    assert (instance->vkCreateDevice(instance->physicalDevices[deviceNumber], &creationInfo, nullptr, &device) == VK_SUCCESS);

    /***********************************************************
     * Enumerate Device Function Pointers
     ***********************************************************/
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
}

VulkanDevice::~VulkanDevice(){
    instance->vkDestroyDevice(device, nullptr);
}

VulkanCommandPool * VulkanDevice::getCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex){
    VulkanCommandPool * commandPool = new VulkanCommandPool(this, flags, queueFamilyIndex);

    return commandPool;
}

int32_t VulkanDevice::getUsableMemoryType(uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProperties){
    int32_t type = -1;
    uint32_t memoryTypeIndex = 0;
    while (type == -1 && memoryTypeIndex < deviceMemoryProperties.memoryTypeCount){
        if ((memoryTypeBits & 1 << memoryTypeIndex) != 0){
            if ((requiredProperties & deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags) == requiredProperties){
                type = memoryTypeIndex;
            }
        }
        memoryTypeIndex++;
    }

    return type;
}

int32_t VulkanDevice::getUsableDeviceQueueFamily(const VkQueueFlags requiredProperties){
    int32_t queueFamily = -1;
    uint32_t queueFamilyIndex = 0;
    while(queueFamilyIndex < deviceQueueFamilyPropertyCount && queueFamily == -1){
        auto queueFamilyProperty = deviceQueueProperties[queueFamilyIndex];

        if((requiredProperties & queueFamilyProperty.queueFlags) == requiredProperties){
            queueFamily = queueFamilyIndex;
        }
        queueFamilyIndex++;
    }

    return queueFamily;
}

// VkImage and VkBuffer collide on VC++
//void VulkanDevice::allocateAndBindMemory(VkBuffer buffer, bool hostVisible){
//    VkDeviceMemory          bufferMemory;
//    VkMemoryAllocateInfo    allocateInfo;
//    VkMemoryRequirements    memoryRequirements;
//
//    // Get Memory Requirements
//    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
//
//    int32_t memoryType = getUsableMemoryType(memoryRequirements.memoryTypeBits, 
//                                                            hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//    assert (memoryType != -1);
//    std::cout << "Memory type for buffer: " << std::dec << memoryType << std::endl;
//
//    // Allocate Memory
//    allocateInfo.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    allocateInfo.pNext              = nullptr;
//    allocateInfo.allocationSize     = memoryRequirements.size;
//    allocateInfo.memoryTypeIndex    = memoryType;
//    assert( vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory) == VK_SUCCESS);
//
//    // Bind Memory for buffer
//    assert( vkBindBufferMemory(device, buffer, bufferMemory, 0) == VK_SUCCESS);
//}

void VulkanDevice::allocateAndBindMemory(VkImage image, bool hostVisible){
    VkDeviceMemory          imageMemory;
    VkMemoryAllocateInfo    allocateInfo;
    VkMemoryRequirements    memoryRequirements;

    // Get Memory Requirements
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    std::cout << "Memory requirements for image: " << std::dec << memoryRequirements.size << std::endl;
    int32_t memoryType = getUsableMemoryType(memoryRequirements.memoryTypeBits, 
                                                            hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    assert (memoryType != -1);
    std::cout << "Memory type for image: " << std::dec << memoryType << std::endl;

    // Allocate Memory
    allocateInfo.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext              = nullptr;
    allocateInfo.allocationSize     = memoryRequirements.size;
    allocateInfo.memoryTypeIndex    = memoryType;
    assert( vkAllocateMemory(device, &allocateInfo, nullptr, &imageMemory) == VK_SUCCESS);

    // Bind Memory for image
    assert( vkBindImageMemory(device, image, imageMemory, 0) == VK_SUCCESS);
}

VulkanDriverInstance::VulkanDriverInstance(std::string applicationName){
    numPhysicalDevices                              = 0;

#if defined (_WIN32) || defined (_WIN64)
    loader = LoadLibrary("vulkan-1.dll");
#elif defined (__linux__)
    loader = dlopen( "libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
#endif

    assert(loader != nullptr);

    VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
    VK_EXPORTED_FUNCTION(vkEnumerateInstanceLayerProperties);
    VK_EXPORTED_FUNCTION(vkEnumerateInstanceExtensionProperties);
    VK_EXPORTED_FUNCTION(vkCreateInstance);

    // Enable layers
    std::vector<const char*> requestedLayers = {"VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_image", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_swapchain"};
    std::vector<const char*> enabledLayers;
    uint32_t layerCount = 0;
    assert(vkEnumerateInstanceLayerProperties(&layerCount, nullptr) == VK_SUCCESS);
    std::vector<VkLayerProperties> layerPropertiesVector(layerCount);
    std::cout << "Found " << layerCount << " layers." << std::endl;
    assert(vkEnumerateInstanceLayerProperties(&layerCount, &layerPropertiesVector[0]) == VK_SUCCESS);
    for (auto properties : layerPropertiesVector){
        std::cout << "Layer Name: " << properties.layerName << std::endl;
        std::cout << "   Layer Spec Version: " << VK_VERSION_MAJOR(properties.specVersion) << "." << VK_VERSION_MINOR(properties.specVersion) << "." << VK_VERSION_PATCH(properties.specVersion) << std::endl;
        std::cout << "   Layer Implementation Version: " << properties.implementationVersion << std::endl;
        std::cout << "   Layer Desription: " << properties.description << std::endl;
        for (auto requestedLayer : requestedLayers){
            if ( strcmp(requestedLayer, properties.layerName) == 0){
                enabledLayers.push_back(requestedLayer);
                break;
            }
        }
    }
    uint32_t enabledLayerCount = enabledLayers.size();

    // Create the instance
    assert (vkCreateInstance != nullptr);
    VkApplicationInfo app;
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pNext = NULL;
    app.pApplicationName = applicationName.c_str();
    app.applicationVersion = 0;
    app.pEngineName = applicationName.c_str();
    app.engineVersion = 0;
    app.apiVersion = VK_API_VERSION_1_0; // Use version supported by NVIDIA

    // Extensions
    std::vector<const char*> requestedExtensions    = { VK_KHR_SURFACE_EXTENSION_NAME };
    std::vector<const char*> requiredExtensions     = { VK_KHR_SURFACE_EXTENSION_NAME };
    std::vector<const char*> enabledExtensions;
    uint32_t requiredExtensionsFound = 0;
    uint32_t extensionCount = 0;
    assert(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) == VK_SUCCESS);
    std::cout << "Found " << extensionCount << " extensions." << std::endl;
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    assert(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &availableExtensions[0]) == VK_SUCCESS);
    for (auto extensionProperties : availableExtensions){
        std::cout << "Extension Name: " << extensionProperties.extensionName << std::endl;
        std::cout << "   Extension Spec Version: " << extensionProperties.specVersion << std::endl;

        for(auto requestedExtension : requestedExtensions){
            if ( strcmp(requestedExtension, extensionProperties.extensionName) == 0 ){
                enabledExtensions.push_back(requestedExtension);
                break;
            }
        }

        for(auto requiredExtension : requiredExtensions){
            if ( strcmp(requiredExtension, extensionProperties.extensionName) == 0 ){
                requiredExtensionsFound++;
                break;
            }
        }
    }
    uint32_t enabledExtensionCount = enabledExtensions.size();
    // Ensure that all required extensions were found
    assert( requiredExtensionsFound == requiredExtensions.size() );

    // Instance creation info
    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext                    = nullptr;
    instanceCreateInfo.flags                    = 0;
    instanceCreateInfo.pApplicationInfo         = &app;
    instanceCreateInfo.enabledLayerCount        = enabledLayerCount;
    instanceCreateInfo.ppEnabledLayerNames      = &enabledLayers[0];
    instanceCreateInfo.enabledExtensionCount    = enabledExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames  = &enabledExtensions[0];

    // Assert if instance creation failed
    assert (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) == VK_SUCCESS);
    assert (instance != VK_NULL_HANDLE);

    // Enumerate instance function pointers
    VK_INSTANCE_FUNCTION(vkGetDeviceProcAddr);
    VK_INSTANCE_FUNCTION(vkDestroyInstance);
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

    enumeratePhysicalDevices();
}

VulkanDriverInstance::~VulkanDriverInstance(){
    // Destroy Physical Devices
    physicalDevices.clear();

    // Remove loader
#if defined (_WIN32) || defined (_WIN64)
	assert(FreeLibrary(loader) != 0); // Non-zero is success on Windows
#elif defined (__linux__)
	assert(dlclose(loader) == 0);
#endif
    loader = nullptr;
}

void VulkanDriverInstance::enumeratePhysicalDevices(bool debugPrint){
    assert (loader != nullptr);

    // Enumerate Physical Devices
    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr) == VK_SUCCESS);
    physicalDevices = std::vector<VkPhysicalDevice>(numPhysicalDevices);

    if (debugPrint){
        std::cout << "Found " << numPhysicalDevices << " Physical Device(s): " << std::endl;
        std::cout << "Physical Devices: " << std::endl;
    }

    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.data()) == VK_SUCCESS);
}