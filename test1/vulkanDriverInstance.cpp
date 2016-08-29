#include <cassert>
#include "vulkanDriverInstance.h"

#define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function); assert( function != nullptr)
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, #function)); assert( function != nullptr)
#define VK_DEVICE_FUNCTION(function) devices[deviceNumber].function = (PFN_##function)( vkGetDeviceProcAddr( devices[deviceNumber].device, #function)); assert( devices[deviceNumber].function != nullptr)



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
    vkEnumerateDeviceLayerProperties                = nullptr;
    vkEnumerateDeviceExtensionProperties            = nullptr;
    vkEnumeratePhysicalDevices                      = nullptr;
    vkGetPhysicalDeviceFeatures                     = nullptr;
    vkGetPhysicalDeviceFormatProperties             = nullptr;
    vkGetPhysicalDeviceImageFormatProperties        = nullptr;
    vkGetPhysicalDeviceMemoryProperties             = nullptr;
    vkGetPhysicalDeviceProperties                   = nullptr;
    vkGetPhysicalDeviceQueueFamilyProperties        = nullptr;
    vkGetPhysicalDeviceSparseImageFormatProperties  = nullptr;

    loader = dlopen( "libvulkan.so", RTLD_NOW);

    if (loader != nullptr){
        VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
        VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr);
        VK_EXPORTED_FUNCTION(vkCreateInstance);
        VK_EXPORTED_FUNCTION(vkDestroyInstance);


        // Create the instance
        assert (vkCreateInstance != nullptr);
        VkApplicationInfo app;
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pNext = NULL;
        app.pApplicationName = applicationName.c_str();
        app.applicationVersion = 0;
        app.pEngineName = applicationName.c_str();
        app.engineVersion = 0;
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = NULL;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &app;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = NULL;
        instanceCreateInfo.enabledExtensionCount = 0;
        instanceCreateInfo.ppEnabledExtensionNames = NULL;

        // Assert if instance creation failed
        assert (vkCreateInstance(&instanceCreateInfo, NULL, &instance) == VK_SUCCESS);

        // Enumerate instance function pointers
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
        std::cout << "   Physical Device " << std::hex << deviceNumber << ": " << std::endl;
        std::cout << "      API Version: " << std::hex << device.deviceProperties.apiVersion << std::endl;
        std::cout << "      Driver Version: " << std::hex << device.deviceProperties.driverVersion << std::endl;
        std::cout << "      Vendor ID: " << std::hex << device.deviceProperties.vendorID << std::endl;
        std::cout << "      Device ID: " << std::hex << device.deviceProperties.deviceID << std::endl;
        // std::cout << "      Device Type: " << std::endl;
        std::cout << "      Device Name: " << device.deviceProperties.deviceName << std::endl;
    }

    // Enumerate Physical Device Queue Family Properties
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceNumber], &queueFamilyPropertyCount, nullptr);
    assert( queueFamilyPropertyCount != 0);

    if (debugPrint){
        std::cout << "      Found " << queueFamilyPropertyCount << " Queue Families: " << std::endl;
    }
    device.deviceQueueProperties = new VkQueueFamilyProperties[queueFamilyPropertyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceNumber], &queueFamilyPropertyCount, device.deviceQueueProperties);

    for (uint32_t queueFamily = 0; queueFamily < queueFamilyPropertyCount; queueFamily++){
        auto queueFamilyProperties = device.deviceQueueProperties[queueFamily];

        if (debugPrint){
            std::cout << "          Queue Family " << queueFamily << ": " << std::endl;
            std::cout << "              Queue Flags: " << std::hex << queueFamilyProperties.queueFlags << std::endl;
            std::cout << "              Queue Count: " << queueFamilyProperties.queueCount << std::endl;
            std::cout << "              Timestamp Valid Bits: " << queueFamilyProperties.timestampValidBits << std::endl;
            std::cout << "              (TODO)Min Image Transfer Granularity: " << std::hex << "" /*queueFamilyProperties.deviceID */<< std::endl;
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
                    properties = firstProperty ? properties : properties + ",\n         ";
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
    VkDeviceQueueCreateInfo queueInfo[queueFamilyPropertyCount];
    for (uint32_t queueFamily = 0; queueFamily < queueFamilyPropertyCount; queueFamily++){
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

    // Create Info
    VkDeviceCreateInfo creationInfo;
    creationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    creationInfo.pNext = nullptr;
    creationInfo.flags = 0; // Reserved
    creationInfo.queueCreateInfoCount = 1;
    creationInfo.pQueueCreateInfos = queueInfo;
    creationInfo.enabledLayerCount = 0;
    creationInfo.ppEnabledExtensionNames = nullptr;
    creationInfo.enabledExtensionCount = 0;
    creationInfo.ppEnabledExtensionNames = nullptr;
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