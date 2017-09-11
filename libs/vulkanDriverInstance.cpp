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

bool VulkanDevice::comparePhysicalDeviceFeatureSets(const VkPhysicalDeviceFeatures * featureSetA, const VkPhysicalDeviceFeatures * featureSetB){
    bool featureSetsEqual = true;

    // Ensure that both pointers are valid
    assert(featureSetA != nullptr && featureSetB != nullptr);

    // Perform comparison
    featureSetsEqual &= (featureSetA->robustBufferAccess == featureSetB->robustBufferAccess);
    featureSetsEqual &= (featureSetA->fullDrawIndexUint32 == featureSetB->fullDrawIndexUint32);
    featureSetsEqual &= (featureSetA->imageCubeArray == featureSetB->imageCubeArray);
    featureSetsEqual &= (featureSetA->independentBlend == featureSetB->independentBlend);
    featureSetsEqual &= (featureSetA->geometryShader == featureSetB->geometryShader);
    featureSetsEqual &= (featureSetA->tessellationShader == featureSetB->tessellationShader);
    featureSetsEqual &= (featureSetA->sampleRateShading == featureSetB->sampleRateShading);
    featureSetsEqual &= (featureSetA->dualSrcBlend == featureSetB->dualSrcBlend);
    featureSetsEqual &= (featureSetA->logicOp == featureSetB->logicOp);
    featureSetsEqual &= (featureSetA->multiDrawIndirect == featureSetB->multiDrawIndirect);
    featureSetsEqual &= (featureSetA->drawIndirectFirstInstance == featureSetB->drawIndirectFirstInstance);
    featureSetsEqual &= (featureSetA->depthClamp == featureSetB->depthClamp);
    featureSetsEqual &= (featureSetA->depthBiasClamp == featureSetB->depthBiasClamp);
    featureSetsEqual &= (featureSetA->fillModeNonSolid == featureSetB->fillModeNonSolid);
    featureSetsEqual &= (featureSetA->depthBounds == featureSetB->depthBounds);
    featureSetsEqual &= (featureSetA->wideLines == featureSetB->wideLines);
    featureSetsEqual &= (featureSetA->largePoints == featureSetB->largePoints);
    featureSetsEqual &= (featureSetA->alphaToOne == featureSetB->alphaToOne);
    featureSetsEqual &= (featureSetA->multiViewport == featureSetB->multiViewport);
    featureSetsEqual &= (featureSetA->samplerAnisotropy == featureSetB->samplerAnisotropy);
    featureSetsEqual &= (featureSetA->textureCompressionETC2 == featureSetB->textureCompressionETC2);
    featureSetsEqual &= (featureSetA->textureCompressionASTC_LDR == featureSetB->textureCompressionASTC_LDR);
    featureSetsEqual &= (featureSetA->textureCompressionBC == featureSetB->textureCompressionBC);
    featureSetsEqual &= (featureSetA->occlusionQueryPrecise == featureSetB->occlusionQueryPrecise);
    featureSetsEqual &= (featureSetA->pipelineStatisticsQuery == featureSetB->pipelineStatisticsQuery);
    featureSetsEqual &= (featureSetA->vertexPipelineStoresAndAtomics == featureSetB->vertexPipelineStoresAndAtomics);
    featureSetsEqual &= (featureSetA->fragmentStoresAndAtomics == featureSetB->fragmentStoresAndAtomics);
    featureSetsEqual &= (featureSetA->shaderTessellationAndGeometryPointSize == featureSetB->shaderTessellationAndGeometryPointSize);
    featureSetsEqual &= (featureSetA->shaderImageGatherExtended == featureSetB->shaderImageGatherExtended);
    featureSetsEqual &= (featureSetA->shaderStorageImageExtendedFormats == featureSetB->shaderStorageImageExtendedFormats);
    featureSetsEqual &= (featureSetA->shaderStorageImageMultisample == featureSetB->shaderStorageImageMultisample);
    featureSetsEqual &= (featureSetA->shaderStorageImageReadWithoutFormat == featureSetB->shaderStorageImageReadWithoutFormat);
    featureSetsEqual &= (featureSetA->shaderStorageImageWriteWithoutFormat == featureSetB->shaderStorageImageWriteWithoutFormat);
    featureSetsEqual &= (featureSetA->shaderUniformBufferArrayDynamicIndexing == featureSetB->shaderUniformBufferArrayDynamicIndexing);
    featureSetsEqual &= (featureSetA->shaderSampledImageArrayDynamicIndexing == featureSetB->shaderSampledImageArrayDynamicIndexing);
    featureSetsEqual &= (featureSetA->shaderStorageBufferArrayDynamicIndexing == featureSetB->shaderStorageBufferArrayDynamicIndexing);
    featureSetsEqual &= (featureSetA->shaderStorageImageArrayDynamicIndexing == featureSetB->shaderStorageImageArrayDynamicIndexing);
    featureSetsEqual &= (featureSetA->shaderClipDistance == featureSetB->shaderClipDistance);
    featureSetsEqual &= (featureSetA->shaderCullDistance == featureSetB->shaderCullDistance);
    featureSetsEqual &= (featureSetA->shaderFloat64 == featureSetB->shaderFloat64);
    featureSetsEqual &= (featureSetA->shaderInt64 == featureSetB->shaderInt64);
    featureSetsEqual &= (featureSetA->shaderInt16 == featureSetB->shaderInt16);
    featureSetsEqual &= (featureSetA->shaderResourceResidency == featureSetB->shaderResourceResidency);
    featureSetsEqual &= (featureSetA->shaderResourceMinLod == featureSetB->shaderResourceMinLod);
    featureSetsEqual &= (featureSetA->sparseBinding == featureSetB->sparseBinding);
    featureSetsEqual &= (featureSetA->sparseResidencyBuffer == featureSetB->sparseResidencyBuffer);
    featureSetsEqual &= (featureSetA->sparseResidencyImage2D == featureSetB->sparseResidencyImage2D);
    featureSetsEqual &= (featureSetA->sparseResidencyImage3D == featureSetB->sparseResidencyImage3D);
    featureSetsEqual &= (featureSetA->sparseResidency2Samples == featureSetB->sparseResidency2Samples);
    featureSetsEqual &= (featureSetA->sparseResidency4Samples == featureSetB->sparseResidency4Samples);
    featureSetsEqual &= (featureSetA->sparseResidency8Samples == featureSetB->sparseResidency8Samples);
    featureSetsEqual &= (featureSetA->sparseResidency16Samples == featureSetB->sparseResidency16Samples);
    featureSetsEqual &= (featureSetA->sparseResidencyAliased == featureSetB->sparseResidencyAliased);
    featureSetsEqual &= (featureSetA->variableMultisampleRate == featureSetB->variableMultisampleRate);
    featureSetsEqual &= (featureSetA->inheritedQueries == featureSetB->inheritedQueries);

    return featureSetsEqual;
}

VkPhysicalDeviceFeatures VulkanDevice::filterPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures * featureSetA, const VkPhysicalDeviceFeatures * featureSetB){
    VkPhysicalDeviceFeatures filteredFeatureSet;

    // Ensure that both pointers are valid
    assert(featureSetA != nullptr && featureSetB != nullptr);

    // TODO: Add support for multiple compare operations using lambdas

    // Perform filtering
    filteredFeatureSet.robustBufferAccess                       = featureSetA->robustBufferAccess & featureSetB->robustBufferAccess;
    filteredFeatureSet.fullDrawIndexUint32                      = featureSetA->fullDrawIndexUint32 & featureSetB->fullDrawIndexUint32;
    filteredFeatureSet.imageCubeArray                           = featureSetA->imageCubeArray & featureSetB->imageCubeArray;
    filteredFeatureSet.independentBlend                         = featureSetA->independentBlend & featureSetB->independentBlend;
    filteredFeatureSet.geometryShader                           = featureSetA->geometryShader & featureSetB->geometryShader;
    filteredFeatureSet.tessellationShader                       = featureSetA->tessellationShader & featureSetB->tessellationShader;
    filteredFeatureSet.sampleRateShading                        = featureSetA->sampleRateShading & featureSetB->sampleRateShading;
    filteredFeatureSet.dualSrcBlend                             = featureSetA->dualSrcBlend & featureSetB->dualSrcBlend;
    filteredFeatureSet.logicOp                                  = featureSetA->logicOp & featureSetB->logicOp;
    filteredFeatureSet.multiDrawIndirect                        = featureSetA->multiDrawIndirect & featureSetB->multiDrawIndirect;
    filteredFeatureSet.drawIndirectFirstInstance                = featureSetA->drawIndirectFirstInstance & featureSetB->drawIndirectFirstInstance;
    filteredFeatureSet.depthClamp                               = featureSetA->depthClamp & featureSetB->depthClamp;
    filteredFeatureSet.depthBiasClamp                           = featureSetA->depthBiasClamp & featureSetB->depthBiasClamp;
    filteredFeatureSet.fillModeNonSolid                         = featureSetA->fillModeNonSolid & featureSetB->fillModeNonSolid;
    filteredFeatureSet.depthBounds                              = featureSetA->depthBounds & featureSetB->depthBounds;
    filteredFeatureSet.wideLines                                = featureSetA->wideLines & featureSetB->wideLines;
    filteredFeatureSet.largePoints                              = featureSetA->largePoints & featureSetB->largePoints;
    filteredFeatureSet.alphaToOne                               = featureSetA->alphaToOne & featureSetB->alphaToOne;
    filteredFeatureSet.multiViewport                            = featureSetA->multiViewport & featureSetB->multiViewport;
    filteredFeatureSet.samplerAnisotropy                        = featureSetA->samplerAnisotropy & featureSetB->samplerAnisotropy;
    filteredFeatureSet.textureCompressionETC2                   = featureSetA->textureCompressionETC2 & featureSetB->textureCompressionETC2;
    filteredFeatureSet.textureCompressionASTC_LDR               = featureSetA->textureCompressionASTC_LDR & featureSetB->textureCompressionASTC_LDR;
    filteredFeatureSet.textureCompressionBC                     = featureSetA->textureCompressionBC & featureSetB->textureCompressionBC;
    filteredFeatureSet.occlusionQueryPrecise                    = featureSetA->occlusionQueryPrecise & featureSetB->occlusionQueryPrecise;
    filteredFeatureSet.pipelineStatisticsQuery                  = featureSetA->pipelineStatisticsQuery & featureSetB->pipelineStatisticsQuery;
    filteredFeatureSet.vertexPipelineStoresAndAtomics           = featureSetA->vertexPipelineStoresAndAtomics & featureSetB->vertexPipelineStoresAndAtomics;
    filteredFeatureSet.fragmentStoresAndAtomics                 = featureSetA->fragmentStoresAndAtomics & featureSetB->fragmentStoresAndAtomics;
    filteredFeatureSet.shaderTessellationAndGeometryPointSize   = featureSetA->shaderTessellationAndGeometryPointSize & featureSetB->shaderTessellationAndGeometryPointSize;
    filteredFeatureSet.shaderImageGatherExtended                = featureSetA->shaderImageGatherExtended & featureSetB->shaderImageGatherExtended;
    filteredFeatureSet.shaderStorageImageExtendedFormats        = featureSetA->shaderStorageImageExtendedFormats & featureSetB->shaderStorageImageExtendedFormats;
    filteredFeatureSet.shaderStorageImageMultisample            = featureSetA->shaderStorageImageMultisample & featureSetB->shaderStorageImageMultisample;
    filteredFeatureSet.shaderStorageImageReadWithoutFormat      = featureSetA->shaderStorageImageReadWithoutFormat & featureSetB->shaderStorageImageReadWithoutFormat;
    filteredFeatureSet.shaderStorageImageWriteWithoutFormat     = featureSetA->shaderStorageImageWriteWithoutFormat & featureSetB->shaderStorageImageWriteWithoutFormat;
    filteredFeatureSet.shaderUniformBufferArrayDynamicIndexing  = featureSetA->shaderUniformBufferArrayDynamicIndexing & featureSetB->shaderUniformBufferArrayDynamicIndexing;
    filteredFeatureSet.shaderSampledImageArrayDynamicIndexing   = featureSetA->shaderSampledImageArrayDynamicIndexing & featureSetB->shaderSampledImageArrayDynamicIndexing;
    filteredFeatureSet.shaderStorageBufferArrayDynamicIndexing  = featureSetA->shaderStorageBufferArrayDynamicIndexing & featureSetB->shaderStorageBufferArrayDynamicIndexing;
    filteredFeatureSet.shaderStorageImageArrayDynamicIndexing   = featureSetA->shaderStorageImageArrayDynamicIndexing & featureSetB->shaderStorageImageArrayDynamicIndexing;
    filteredFeatureSet.shaderClipDistance                       = featureSetA->shaderClipDistance & featureSetB->shaderClipDistance;
    filteredFeatureSet.shaderCullDistance                       = featureSetA->shaderCullDistance & featureSetB->shaderCullDistance;
    filteredFeatureSet.shaderFloat64                            = featureSetA->shaderFloat64 & featureSetB->shaderFloat64;
    filteredFeatureSet.shaderInt64                              = featureSetA->shaderInt64 & featureSetB->shaderInt64;
    filteredFeatureSet.shaderInt16                              = featureSetA->shaderInt16 & featureSetB->shaderInt16;
    filteredFeatureSet.shaderResourceResidency                  = featureSetA->shaderResourceResidency & featureSetB->shaderResourceResidency;
    filteredFeatureSet.shaderResourceMinLod                     = featureSetA->shaderResourceMinLod & featureSetB->shaderResourceMinLod;
    filteredFeatureSet.sparseBinding                            = featureSetA->sparseBinding & featureSetB->sparseBinding;
    filteredFeatureSet.sparseResidencyBuffer                    = featureSetA->sparseResidencyBuffer & featureSetB->sparseResidencyBuffer;
    filteredFeatureSet.sparseResidencyImage2D                   = featureSetA->sparseResidencyImage2D & featureSetB->sparseResidencyImage2D;
    filteredFeatureSet.sparseResidencyImage3D                   = featureSetA->sparseResidencyImage3D & featureSetB->sparseResidencyImage3D;
    filteredFeatureSet.sparseResidency2Samples                  = featureSetA->sparseResidency2Samples & featureSetB->sparseResidency2Samples;
    filteredFeatureSet.sparseResidency4Samples                  = featureSetA->sparseResidency4Samples & featureSetB->sparseResidency4Samples;
    filteredFeatureSet.sparseResidency8Samples                  = featureSetA->sparseResidency8Samples & featureSetB->sparseResidency8Samples;
    filteredFeatureSet.sparseResidency16Samples                 = featureSetA->sparseResidency16Samples & featureSetB->sparseResidency16Samples;
    filteredFeatureSet.sparseResidencyAliased                   = featureSetA->sparseResidencyAliased & featureSetB->sparseResidencyAliased;
    filteredFeatureSet.variableMultisampleRate                  = featureSetA->variableMultisampleRate & featureSetB->variableMultisampleRate;
    filteredFeatureSet.inheritedQueries                         = featureSetA->inheritedQueries & featureSetB->inheritedQueries;

    return filteredFeatureSet;
}
VulkanDevice::VulkanDevice(VulkanDriverInstance * __instance, uint32_t __deviceNumber, const VkPhysicalDeviceFeatures * requestedFeatures, const VkPhysicalDeviceFeatures * requiredFeatures, bool debugPrint){
    instance = __instance;
    deviceNumber = __deviceNumber;
    assert(instance != nullptr);
    assert (deviceNumber < instance->numPhysicalDevices);

    // Get Properties
    instance->vkGetPhysicalDeviceProperties(instance->physicalDevices[deviceNumber], &deviceProperties);

    // Get Device Features
    instance->vkGetPhysicalDeviceFeatures(instance->physicalDevices[deviceNumber], &deviceFeatures);

    // Handle Features
    VkPhysicalDeviceFeatures appliedFeatures;
    if(requestedFeatures != nullptr){
        // Required Features
        if(requiredFeatures != nullptr){
            VkPhysicalDeviceFeatures requiredFeatureSetIntersection = VulkanDevice::filterPhysicalDeviceFeatures(&deviceFeatures, requiredFeatures);
            if(VulkanDevice::comparePhysicalDeviceFeatureSets(&requiredFeatureSetIntersection, requiredFeatures) != true){
                std::cout << "Required features not supported on this device! Exiting." << std::endl;
                exit(-1);
            }

            // Requested features must be a superset of required features
            VkPhysicalDeviceFeatures requestedFeatureSetIntersection = VulkanDevice::filterPhysicalDeviceFeatures(requestedFeatures, requiredFeatures);
            assert(VulkanDevice::comparePhysicalDeviceFeatureSets(&requestedFeatureSetIntersection, requiredFeatures) == true);
        }

        appliedFeatures = VulkanDevice::filterPhysicalDeviceFeatures(requestedFeatures, &deviceFeatures);
    }else if(requiredFeatures != nullptr){
        appliedFeatures = VulkanDevice::filterPhysicalDeviceFeatures(&deviceFeatures, requiredFeatures);
        if(VulkanDevice::comparePhysicalDeviceFeatureSets(&appliedFeatures, requiredFeatures) != true){
            std::cout << "Required features not supported on this device! Exiting." << std::endl;
            exit(-1);
        }
    }

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
            properties += std::to_string(memoryType.heapIndex);
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
    creationInfo.pEnabledFeatures =  ((requiredFeatures != nullptr || requestedFeatures != nullptr) ? &appliedFeatures : nullptr);

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
}

VulkanDevice::~VulkanDevice(){
    // Clean up descriptor pools
    for(auto descriptorPool : descriptorPools){
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }
    instance->vkDestroyDevice(device, nullptr);
}

VkDescriptorPool * VulkanDevice::getDescriptorPool(const std::vector<VkDescriptorPoolSize>& descriptorSizes){
    VkDescriptorPool * pool = new VkDescriptorPool();

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = descriptorSizes.size();
    poolInfo.poolSizeCount = descriptorSizes.size();
    poolInfo.pPoolSizes = &descriptorSizes[0];

    assert(vkCreateDescriptorPool(device, &poolInfo, nullptr, pool) == VK_SUCCESS);
    descriptorPools.push_back(*pool);
    return pool;
}

VulkanCommandPool * VulkanDevice::getCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex){
    VulkanCommandPool * commandPool = new VulkanCommandPool(this, flags, queueFamilyIndex);

    return commandPool;
}

VkPhysicalDevice VulkanDevice::getPhysicalDevice(){
    return instance->physicalDevices.at(deviceNumber);
}

uint32_t VulkanDevice::getUsableMemoryType(uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProperties){
    uint32_t type = (std::numeric_limits<uint32_t>::max)();
    uint32_t memoryTypeIndex = 0;
    while (type == (std::numeric_limits<uint32_t>::max)() && memoryTypeIndex < deviceMemoryProperties.memoryTypeCount){
        if ((memoryTypeBits & 1 << memoryTypeIndex) != 0){
            if ((requiredProperties & deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags) == requiredProperties){
                type = memoryTypeIndex;
            }
        }
        memoryTypeIndex++;
    }

    return type;
}

uint32_t VulkanDevice::getUsableDeviceQueueFamily(const VkQueueFlags requiredProperties){
    uint32_t queueFamily = (std::numeric_limits<uint32_t>::max)();
    uint32_t queueFamilyIndex = 0;
    while(queueFamilyIndex < deviceQueueFamilyPropertyCount && queueFamily == (std::numeric_limits<uint32_t>::max)()){
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

VkDeviceMemory VulkanDevice::allocateAndBindMemory(VkImage image, bool hostVisible){
    VkDeviceMemory          imageMemory;
    VkMemoryAllocateInfo    allocateInfo;
    VkMemoryRequirements    memoryRequirements;

    // Get Memory Requirements
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    std::cout << "Memory requirements for image: " << std::dec << memoryRequirements.size << std::endl;
    uint32_t memoryType = getUsableMemoryType(memoryRequirements.memoryTypeBits, 
                                                            hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    assert (memoryType != (std::numeric_limits<uint32_t>::max)());
    std::cout << "Memory type for image: " << std::dec << memoryType << std::endl;

    // Allocate Memory
    allocateInfo.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext              = nullptr;
    allocateInfo.allocationSize     = memoryRequirements.size;
    allocateInfo.memoryTypeIndex    = memoryType;
    assert( vkAllocateMemory(device, &allocateInfo, nullptr, &imageMemory) == VK_SUCCESS);

    // Bind Memory for image
    assert( vkBindImageMemory(device, image, imageMemory, 0) == VK_SUCCESS);

    return imageMemory;
}

VkFormat VulkanDevice::getSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    VkFormat format = VK_FORMAT_UNDEFINED;

    std::cout << "Requested Tiling: " << (tiling == VK_IMAGE_TILING_LINEAR ? "Linear" : "Optimal") << std::endl;
    std::cout << "Requested Features: " << std::hex << features << std::endl;

    for(auto candidate : candidates){
        // Get Format Properties
        VkFormatProperties properties;
        instance->vkGetPhysicalDeviceFormatProperties(instance->physicalDevices[deviceNumber], candidate, &properties);
        VkFormatFeatureFlags featureIntersection;

        std::cout << "Candidate Format: " << candidate << std::endl;
        std::cout << "Properties: " << std::endl;
        std::cout << "   Linear Tiling Features: " << std::hex << properties.linearTilingFeatures << std::endl;
        std::cout << "   Optimal Tiling Features: " << std::hex << properties.optimalTilingFeatures << std::endl;
        std::cout << "   Buffer Features: " << std::hex << properties.bufferFeatures << std::endl;


        if(tiling == VK_IMAGE_TILING_LINEAR){
            featureIntersection = properties.linearTilingFeatures;
        }else if (tiling == VK_IMAGE_TILING_OPTIMAL){
            featureIntersection = properties.optimalTilingFeatures;
        }else{
            throw std::runtime_error("Invalid tiling mode requested!");
        }

        featureIntersection &= features;
        if(featureIntersection == features){
            format = candidate;
            break;
        }
    }

    return format;
}

VkSampleCountFlagBits VulkanDevice::requestSupportedSampleFlags(uint32_t sampleCount){
    VkSampleCountFlagBits sampleCountFlag;
    VkSampleCountFlags sampleCountLimits = deviceImageFormatProperties.sampleCounts;
    // Convert sample count to enumeration
    switch(sampleCount){
        case 1:
            sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
            break;
        case 2:
            sampleCountFlag = VK_SAMPLE_COUNT_2_BIT;
            break;
        case 4:
            sampleCountFlag = VK_SAMPLE_COUNT_4_BIT;
            break;
        case 8:
            sampleCountFlag = VK_SAMPLE_COUNT_8_BIT;
            break;
        case 16:
            sampleCountFlag = VK_SAMPLE_COUNT_16_BIT;
            break;
        case 32:
            sampleCountFlag = VK_SAMPLE_COUNT_32_BIT;
            break;
        case 64:
            sampleCountFlag = VK_SAMPLE_COUNT_64_BIT;
            break;
        default:
            sampleCountFlag = VK_SAMPLE_COUNT_1_BIT;
    }

    std::cout << "Request sample count {" << std::hex << sampleCountFlag << "}." << std::endl;
    std::cout << "Sample count limits {" << std::hex << sampleCountLimits << "}." << std::endl;
    // If selected sample count isn't supported, find the closest supported count
    if((sampleCountFlag & sampleCountLimits) != sampleCountFlag){
        std::cout << "Request sample count is not supported." << std::endl;
        while((sampleCountFlag & sampleCountLimits) != sampleCountFlag){
            sampleCountFlag = (VkSampleCountFlagBits)((uint32_t)sampleCountFlag >> 1);
        }
        std::cout << "Falling back to closest supported count {" << sampleCountFlag << "}" << std::endl;
    }

    return sampleCountFlag;
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

#if defined (_WIN32) || defined (_WIN64)
    requestedExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined (__linux__)
    requestedExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

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