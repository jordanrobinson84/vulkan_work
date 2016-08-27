#include <cassert>
#include "vulkanDriverInstance.h"

#define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function); assert( function != nullptr)
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, #function)); assert( function != nullptr)
#define VK_DEVICE_FUNCTION(function, deviceNumber) devices[deviceNumber].function = (PFN_##function)( vkGetDeviceProcAddr( *devices[deviceNumber].device, #function)); assert( devices[deviceNumber].function != nullptr)

VulkanDriverInstance::VulkanDriverInstance(std::string applicationName){
    // OS not used yet
    loader                                      = nullptr;
    //Exported functions
    instance                                    = nullptr;
    vkGetInstanceProcAddr                       = nullptr;
    vkCreateInstance                            = nullptr;

    // Instance variables and functions
    numPhysicalDevices                          = -1;
    physicalDevices                             = nullptr;
    physicalDeviceQueueProperties               = nullptr;
    devices                                     = nullptr;
    vkGetPhysicalDeviceProperties               = nullptr;
    vkGetPhysicalDeviceQueueFamilyProperties    = nullptr;
    vkCreateDevice                              = nullptr;
    vkDestroyDevice                             = nullptr;
    vkEnumerateInstanceLayerProperties          = nullptr;

    loader = dlopen( "libvulkan.so", RTLD_NOW);

    if (loader != nullptr){
        VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr, devices[deviceNumber]);
        VK_EXPORTED_FUNCTION(vkGetDeviceProcAddr, devices[deviceNumber]);
        VK_EXPORTED_FUNCTION(vkCreateInstance, devices[deviceNumber]);
        VK_EXPORTED_FUNCTION(vkDestroyInstance, devices[deviceNumber]);


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
        VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices, devices[deviceNumber]);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties, devices[deviceNumber]);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties, devices[deviceNumber]);
        VK_INSTANCE_FUNCTION(vkCreateDevice, devices[deviceNumber]);
        VK_INSTANCE_FUNCTION(vkDestroyDevice, devices[deviceNumber]);
        // VK_INSTANCE_FUNCTION(vkEnumerateInstanceLayerProperties, devices[deviceNumber]);

        enumeratePhysicalDevices();
    }
}

VulkanDriverInstance::~VulkanDriverInstance(){
    // Destroy Devices
    for (auto deviceObject : devices){
        if (deviceObject.created){
            assert (vkDestroyDevice(physicalDevice, &creationInfo, nullptr, deviceObject.device) == VK_SUCCESS);
            deviceObject.created = false;
            delete deviceObject.device;
        }
    }

    // Destroy Queue Family Properties
    for (auto queueFamilyPropertyPtr : physicalDeviceQueueProperties){
        if (queueFamilyPropertyPtr != nullptr){
            delete[] queueFamilyPropertyPtr;
        }
    }
    // Destroy Physical Device Queue Properties
    delete[] physicalDeviceQueueProperties;
    // Destroy Physical Device Properties
    delete[] physicalDeviceProperties;
    // Destroy Physical Device
    delete[] physicalDevices;

    // Remove loader
    loader = nullptr;
    assert(dlclose("libvulkan.so") == 0);
}

void VulkanDriverInstance::enumeratePhysicalDevices(){
    assert (loader != nullptr);

    // Enumerate Physical Devices
    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr) == VK_SUCCESS);
    physicalDevices = new VkPhysicalDevice[numPhysicalDevices];
    devices = new VulkanDevice[numPhysicalDevices];
    physicalDeviceProperties = new VkPhysicalDeviceProperties[numPhysicalDevices];
    physicalDeviceQueueProperties = new VkQueueFamilyPropertiesPtr[numPhysicalDevices];
    std::cout << "Found " << numPhysicalDevices << " Physical Device(s): " << std::endl;
    assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices) == VK_SUCCESS);
    std::cout << "Physical Devices: " << std::endl;
    for (uint32_t dev = 0; dev < numPhysicalDevices; dev++){
        // Get Properties
        auto physicalDevice = physicalDevices[dev];
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties[dev]);

        std::cout << "   Physical Device " << std::hex << dev << ": " << std::endl;
        std::cout << "      API Version: " << std::hex << physicalDeviceProperties[dev].apiVersion << std::endl;
        std::cout << "      Driver Version: " << std::hex << physicalDeviceProperties[dev].driverVersion << std::endl;
        std::cout << "      Vendor ID: " << std::hex << physicalDeviceProperties[dev].vendorID << std::endl;
        std::cout << "      Device ID: " << std::hex << physicalDeviceProperties[dev].deviceID << std::endl;
        // std::cout << "      Device Type: " << std::endl;
        std::cout << "      Device Name: " << physicalDeviceProperties[dev].deviceName << std::endl;

        // Enumerate Physical Device Queue Family Properties
        uint32_t queueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, NULL);
        assert( queueFamilyPropertyCount != 0);
        std::cout << "      Found " << queueFamilyPropertyCount << " Queue Families: " << std::endl;
        physicalDeviceQueueProperties[dev] = new VkQueueFamilyProperties[queueFamilyPropertyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, physicalDeviceQueueProperties[dev]);

        for (uint32_t queueFamily = 0; queueFamily < queueFamilyPropertyCount; queueFamily++){
            auto queueFamilyProperties = physicalDeviceQueueProperties[dev][queueFamily];

            std::cout << "          Queue Family " << queueFamily << ": " << std::endl;
            std::cout << "              Queue Flags: " << std::hex << queueFamilyProperties.queueFlags << std::endl;
            std::cout << "              Queue Count: " << queueFamilyProperties.queueCount << std::endl;
            std::cout << "              Timestamp Valid Bits: " << queueFamilyProperties.timestampValidBits << std::endl;
            std::cout << "              (TODO)Min Image Transfer Granularity: " << std::hex << "" /*queueFamilyProperties.deviceID */<< std::endl;
        }
    }
}

void VulkanDriverInstance::setupDevice(uint32_t deviceNumber){
    assert (deviceNumber >= 0 && deviceNumber < numPhysicalDevices);

    // Create device
    devices[deviceNumber].device = new VkDevice();

    // Device Queue Create Info
    VkDeviceQueueCreateInfo queueInfo;
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.flags = 0;
    queueInfo.queueFamilyIndex = 0;
    queueInfo.queueCount = physicalDeviceQueueProperties[deviceNumber][0].queueCount;
    float * queuePriorities = new float[queueInfo.queueCount];
    for (uint32_t i = 0; i < queueInfo.queueCount; i++){
        queuePriorities[i] = 1.0; // Leave at same priority for now
    }
    queueInfo.pQueuePriorities = queuePriorities;

    // Create Info
    VkDeviceCreateInfo creationInfo;
    creationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    creationInfo.pNext = nullptr;
    creationInfo.flags = 0; // Reserved
    creationInfo.queueCreateInfoCount = 1;
    creationInfo.pQueueCreateInfos = &queueInfo;
    creationInfo.enabledLayerCount = 0;
    creationInfo.ppEnabledExtensionNames = nullptr;
    creationInfo.enabledExtensionCount = 0;
    creationInfo.ppEnabledExtensionNames = nullptr;
    creationInfo.pEnabledFeatures = nullptr;

    // Create Device
    assert (vkCreateDevice(physicalDevice, &creationInfo, nullptr, devices[deviceNumber].device) == VK_SUCCESS);

    /***********************************************************
     * Enumerate Device Function Pointers
     ***********************************************************/
    VK_DEVICE_FUNCTION(vkAllocateCommandBuffers, deviceNumber);
    VK_DEVICE_FUNCTION(vkAllocateDescriptorSets, deviceNumber);
    VK_DEVICE_FUNCTION(vkAllocateMemory, deviceNumber);
    VK_DEVICE_FUNCTION(vkBindBufferMemory, deviceNumber);
    VK_DEVICE_FUNCTION(vkBindImageMemory, deviceNumber);
    // Creation
    VK_DEVICE_FUNCTION(vkCreateBuffer, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateBufferView, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateCommandPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateComputePipelines, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateDescriptorPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateDescriptorSetLayout, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateEvent, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateFence, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateFramebuffer, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateGraphicsPipelines, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateImage, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateImageView, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreatePipelineCache, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreatePipelineLayout, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateQueryPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateRenderPass, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateSampler, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateSemaphore, deviceNumber);
    VK_DEVICE_FUNCTION(vkCreateShaderModule, deviceNumber);

    // Destruction
    VK_DEVICE_FUNCTION(vkDestroyBuffer, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyBufferView, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyCommandPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyDescriptorPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyEvent, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyFence, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyFramebuffer, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyImage, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyImageView, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyPipeline, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyPipelineCache, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyPipelineLayout, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyQueryPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyRenderPass, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroySampler, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroySemaphore, deviceNumber);
    VK_DEVICE_FUNCTION(vkDestroyShaderModule, deviceNumber);

    VK_DEVICE_FUNCTION(vkDeviceWaitIdle, deviceNumber);
    VK_DEVICE_FUNCTION(vkFlushMappedMemoryRanges, deviceNumber);

    // Free
    VK_DEVICE_FUNCTION(vkFreeCommandBuffers, deviceNumber);
    VK_DEVICE_FUNCTION(vkFreeDescriptorSets, deviceNumber);
    VK_DEVICE_FUNCTION(vkFreeMemory, deviceNumber);

    // Getters
    VK_DEVICE_FUNCTION(vkGetBufferMemoryRequirements, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetDeviceMemoryCommitment, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetDeviceQueue, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetEventStatus, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetFenceStatus, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetImageMemoryRequirement, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetImageSparseMemoryRequirements, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetImageSubresourceLayout, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetPipelineCacheData, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetQueryPoolResults, deviceNumber);
    VK_DEVICE_FUNCTION(vkGetRenderAreaGranularity, deviceNumber);

    VK_DEVICE_FUNCTION(vkInvalidateMappedMemoryRanges, deviceNumber);
    VK_DEVICE_FUNCTION(vkMapMemory, deviceNumber);
    VK_DEVICE_FUNCTION(vkMergePipelineCaches, deviceNumber);

    // Reset
    VK_DEVICE_FUNCTION(vkResetCommandPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkResetDescriptorPool, deviceNumber);
    VK_DEVICE_FUNCTION(vkResetEvent, deviceNumber);
    VK_DEVICE_FUNCTION(vkResetFences, deviceNumber);

    VK_DEVICE_FUNCTION(vkSetEvent, deviceNumber);
    VK_DEVICE_FUNCTION(vkUnmapMemory, deviceNumber);
    VK_DEVICE_FUNCTION(vkUpdateDescriptorStates, deviceNumber);
    VK_DEVICE_FUNCTION(vkWaitForFences, deviceNumber);

    // Set created flag
    devices[deviceNumber].created = true;
}