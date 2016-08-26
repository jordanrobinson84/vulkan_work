#include <cassert>
#include "vulkanDriverInstance.h"

#define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function); assert( function != nullptr)
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, #function)); assert( function != nullptr)
#define VK_DEVICE_FUNCTION(function, deviceObject) deviceObject.function = (PFN_##function)( vkGetDeviceProcAddr( *deviceObject.device, #function)); assert( deviceObject.function != nullptr)

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

    // Device variables and functions
    // vkDeviceWaitIdle                            = nullptr;
    // vkEnumerateDeviceLayerProperties            = nullptr;

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
        VK_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties);
        VK_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
        VK_INSTANCE_FUNCTION(vkCreateDevice);
        VK_INSTANCE_FUNCTION(vkDestroyDevice);
        // VK_INSTANCE_FUNCTION(vkEnumerateInstanceLayerProperties);

        enumeratePhysicalDevices();
    }
}

VulkanDriverInstance::~VulkanDriverInstance(){
    // Destroy Devices
    for (auto deviceObject : devices){
        if (deviceObject.created){
            assert (vkDestroyDevice(physicalDevice, &creationInfo, nullptr, deviceObject.device) == VK_SUCCESS);
            deviceObject.created = false;
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

    // Enumerate Device
    assert (vkCreateDevice(physicalDevice, &creationInfo, nullptr, devices[deviceNumber].device) == VK_SUCCESS);

    // Enumerate Device Function Pointers
    VK_DEVICE_FUNCTION(vkDeviceWaitIdle, devices[deviceNumber]);
    VK_DEVICE_FUNCTION(vkCreateImage, devices[deviceNumber]);
    // VK_DEVICE_FUNCTION(vkEnumerateDeviceLayerProperties, devices[deviceNumber]);

    // Set created flag
    devices[deviceNumber].created = true;
}