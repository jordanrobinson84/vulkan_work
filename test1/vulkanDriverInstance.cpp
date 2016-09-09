#include <cassert>
#include "vulkanDriverInstance.h"

#define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function); assert( function != nullptr)
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, #function)); assert( function != nullptr)
#define VK_DEVICE_FUNCTION(function) devices[deviceNumber].function = (PFN_##function)( vkGetDeviceProcAddr( devices[deviceNumber].device, #function)); assert( devices[deviceNumber].function != nullptr)

#define VK_MAJOR_VERSION(apiVersion) (apiVersion >> 22)
#define VK_MINOR_VERSION(apiVersion) ((apiVersion >> 12) & 0x3FF)
#define VK_PATCH_VERSION(apiVersion) (apiVersion & 0xFFF)
#define VK_VERSION_ENCODING(major, minor, patch) ((major & 0x3FF) << 22) | ((minor & 0x3FF) << 12) | (patch & 0xFFF)

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

VulkanDriverInstance::VulkanDriverInstance(std::string applicationName){
    // OS not used yet
    loader                                          = nullptr;
    //Exported functions
    instance                                        = nullptr;
    vkGetInstanceProcAddr                           = nullptr;
    vkCreateInstance                                = nullptr;

    // Instance variables and functions
    numPhysicalDevices                              = 0;
    vkCreateDevice                                  = nullptr;
    vkDestroyDevice                                 = nullptr;
    vkEnumerateInstanceLayerProperties              = nullptr;
    vkEnumerateInstanceExtensionProperties          = nullptr;
    vkEnumeratePhysicalDevices                      = nullptr;
    vkGetPhysicalDeviceFeatures                     = nullptr;
    vkGetPhysicalDeviceFormatProperties             = nullptr;
    vkGetPhysicalDeviceImageFormatProperties        = nullptr;
    vkGetPhysicalDeviceMemoryProperties             = nullptr;
    vkGetPhysicalDeviceProperties                   = nullptr;
    vkGetPhysicalDeviceQueueFamilyProperties        = nullptr;
    vkGetPhysicalDeviceSparseImageFormatProperties  = nullptr;

    loader = dlopen( "libvulkan.so", RTLD_LOCAL | RTLD_NOW);

    if (loader != nullptr){
        VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
        VK_EXPORTED_FUNCTION(vkEnumerateInstanceLayerProperties);
        VK_EXPORTED_FUNCTION(vkEnumerateInstanceExtensionProperties);
        VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr);
        VK_EXPORTED_FUNCTION(vkCreateInstance);
        VK_EXPORTED_FUNCTION(vkDestroyInstance);

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
            std::cout << "   Layer Spec Version: " << VK_MAJOR_VERSION(properties.specVersion) << "." << VK_MINOR_VERSION(properties.specVersion) << "." << VK_PATCH_VERSION(properties.specVersion) << std::endl;
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
        app.apiVersion = 0; // Use latest version

        // Extensions
        std::vector<const char*> requestedExtensions = {VK_KHR_XCB_SURFACE_EXTENSION_NAME, "VK_KHR_surface"};
        std::vector<const char*> enabledExtensions;
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
        }
        uint32_t enabledExtensionCount = enabledExtensions.size();
        // delete[] availableExtensions;


        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = NULL;
        instanceCreateInfo.flags                    = 0;
        instanceCreateInfo.pApplicationInfo         = &app;
        instanceCreateInfo.enabledLayerCount        = enabledLayerCount;
        instanceCreateInfo.ppEnabledLayerNames      = &enabledLayers[0];
        instanceCreateInfo.enabledExtensionCount    = enabledExtensionCount;
        instanceCreateInfo.ppEnabledExtensionNames  = &enabledExtensions[0];

        // Assert if instance creation failed
        assert (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) == VK_SUCCESS);

        // Enumerate instance function pointers
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

        // Window System Integration - Most functions are only statically linked
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);

        enumeratePhysicalDevices();
    }
}

VulkanDriverInstance::~VulkanDriverInstance(){
    // Destroy Devices
    for (auto deviceObject : devices){
        if (deviceObject.created){
            vkDestroyDevice(deviceObject.device, nullptr);
            deviceObject.created = false;
        }
    }

    // Destroy Physical Devices
    physicalDevices.clear();

    // Remove loader
    assert(dlclose(loader) == 0);
    loader = nullptr;
}

void VulkanDriverInstance::enumeratePhysicalDevices(bool debugPrint){
    assert (loader != nullptr);

    // Enumerate Physical Devices
    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr) == VK_SUCCESS);
    physicalDevices = std::vector<VkPhysicalDevice>(numPhysicalDevices);
    devices = std::vector<VulkanDevice>(numPhysicalDevices);

    if (debugPrint){
        std::cout << "Found " << numPhysicalDevices << " Physical Device(s): " << std::endl;
        std::cout << "Physical Devices: " << std::endl;
    }

    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.data()) == VK_SUCCESS);
}

void VulkanDriverInstance::setupDevice(uint32_t deviceNumber, bool debugPrint){
    assert (deviceNumber >= 0 && deviceNumber < numPhysicalDevices);
    VulkanDevice &device = devices[deviceNumber];

    // Get Properties
    vkGetPhysicalDeviceProperties(physicalDevices[deviceNumber], &device.deviceProperties);

    if (debugPrint){
        uint32_t majorVersion = VK_MAJOR_VERSION(device.deviceProperties.apiVersion);
        uint32_t minorVersion = VK_MINOR_VERSION(device.deviceProperties.apiVersion);
        uint32_t patchVersion = VK_PATCH_VERSION(device.deviceProperties.apiVersion);

        std::cout << "   Physical Device " << std::hex << deviceNumber << ": " << std::endl;
        std::cout << "      API Version: " << std::dec << majorVersion << "." << minorVersion << "." << patchVersion << std::endl;
        std::cout << "      Driver Version: " << std::hex << device.deviceProperties.driverVersion << std::endl;
        std::cout << "      Vendor ID: " << std::hex << device.deviceProperties.vendorID << std::endl;
        std::cout << "      Device ID: " << std::hex << device.deviceProperties.deviceID << std::endl;

        std::string deviceTypeString = ""; 
        switch(device.deviceProperties.deviceType){
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
        std::cout << "      Device Name: " << device.deviceProperties.deviceName << std::endl;
    }

    // Enumerate Physical Device Queue Family Properties
    device.deviceQueueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceNumber], &device.deviceQueueFamilyPropertyCount, nullptr);
    assert( device.deviceQueueFamilyPropertyCount != 0);

    if (debugPrint){
        std::cout << "      Found " << device.deviceQueueFamilyPropertyCount << " Queue Families: " << std::endl;
    }
    device.deviceQueueProperties = new VkQueueFamilyProperties[device.deviceQueueFamilyPropertyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceNumber], &device.deviceQueueFamilyPropertyCount, device.deviceQueueProperties);

    for (uint32_t queueFamily = 0; queueFamily < device.deviceQueueFamilyPropertyCount; queueFamily++){
        auto queueFamilyProperties = device.deviceQueueProperties[queueFamily];

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
    vkGetPhysicalDeviceMemoryProperties(physicalDevices[deviceNumber], &device.deviceMemoryProperties);

    if (debugPrint){
        // Print Memory Properties
        std::cout << "Memory Properties for Device " << deviceNumber << ": " << std::endl;
        std::cout << "   Memory Types: " << std::endl;
        for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < device.deviceMemoryProperties.memoryTypeCount; memoryTypeIndex++){
            VkMemoryType memoryType = device.deviceMemoryProperties.memoryTypes[memoryTypeIndex];
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
        for (uint32_t memoryHeapIndex = 0; memoryHeapIndex < device.deviceMemoryProperties.memoryHeapCount; memoryHeapIndex++){
            VkMemoryHeap memoryHeap = device.deviceMemoryProperties.memoryHeaps[memoryHeapIndex];
            std::cout << "      Memory Heap " << memoryHeapIndex << ": " << memoryHeap.size << std::endl;
        }
    }

    // Device Queue Create Info
    VkDeviceQueueCreateInfo queueInfo[device.deviceQueueFamilyPropertyCount];
    for (uint32_t queueFamily = 0; queueFamily < device.deviceQueueFamilyPropertyCount; queueFamily++){
        auto queueFamilyProperties = device.deviceQueueProperties[queueFamily];
        
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
    assert(vkEnumerateDeviceExtensionProperties(physicalDevices[deviceNumber], nullptr, &extensionCount, nullptr) == VK_SUCCESS);
    std::cout << "Found " << extensionCount << " extensions." << std::endl;
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    assert(vkEnumerateDeviceExtensionProperties(physicalDevices[deviceNumber], nullptr, &extensionCount, &availableExtensions[0]) == VK_SUCCESS);
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
    creationInfo.pQueueCreateInfos = queueInfo;
    creationInfo.enabledLayerCount = 0;
    creationInfo.ppEnabledLayerNames = nullptr;
    creationInfo.enabledExtensionCount = 1;
    creationInfo.ppEnabledExtensionNames = &enabledExtensions[0];
    creationInfo.pEnabledFeatures = nullptr;

    // Create Device
    assert (vkCreateDevice(physicalDevices[deviceNumber], &creationInfo, nullptr, &devices[deviceNumber].device) == VK_SUCCESS);

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
    VK_DEVICE_FUNCTION(vkResetCommandPool);
    VK_DEVICE_FUNCTION(vkResetDescriptorPool);
    VK_DEVICE_FUNCTION(vkResetEvent);
    VK_DEVICE_FUNCTION(vkResetFences);

    VK_DEVICE_FUNCTION(vkSetEvent);
    VK_DEVICE_FUNCTION(vkUnmapMemory);
    VK_DEVICE_FUNCTION(vkUpdateDescriptorSets);
    VK_DEVICE_FUNCTION(vkWaitForFences);

    // Set created flag
    devices[deviceNumber].created = true;
}

VulkanDevice * VulkanDriverInstance::getDevice(uint32_t deviceNumber, bool debugPrint){
    assert (deviceNumber >= 0 && deviceNumber < numPhysicalDevices);

    // Set up if it hasn't already been done
    if (!devices[deviceNumber].created){
        setupDevice(deviceNumber, debugPrint);
    }

    return &devices[deviceNumber];
}