#include <cassert>
#include "vulkanDriverInstance.h"

#define VK_EXPORTED_FUNCTION(function) function = (PFN_##function)dlsym(loader, #function); assert( function != nullptr)
#define VK_GLOBAL_FUNCTION(function) PFN_##function function
#define VK_INSTANCE_FUNCTION(function) function = (PFN_##function)( vkGetInstanceProcAddr( instance, #function)); assert( function != nullptr)
// #define VK_DEVICE_FUNCTION(function) PFN_##function = (PFN_##function)( vkGetDeviceProcAddr( instance, #function)); assert( function != nullptr)

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
    vkGetPhysicalDeviceProperties               = nullptr;
    vkGetPhysicalDeviceQueueFamilyProperties    = nullptr;
    vkEnumerateInstanceLayerProperties          = nullptr;
    vkEnumerateDeviceLayerProperties            = nullptr;

    // Device variables and functions
    vkCreateDevice                              = nullptr;
    vkDeviceWaitIdle                            = nullptr;
    vkDestroyDevice                             = nullptr;

    loader = dlopen( "libvulkan.so", RTLD_NOW);

    if (loader != nullptr){
        VK_EXPORTED_FUNCTION(vkGetInstanceProcAddr);
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
        // VK_INSTANCE_FUNCTION(vkEnumerateInstanceLayerProperties);

        // Enumerate Physical Devices
        assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr) == VK_SUCCESS);
        physicalDevices = new VkPhysicalDevice[numPhysicalDevices];
        physicalDeviceQueueProperties = new VkQueueFamilyPropertiesPtr[numPhysicalDevices];
        std::cout << "Found " << numPhysicalDevices << " Physical Device(s): " << std::endl;
        assert (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices) == VK_SUCCESS);
        std::cout << "Physical Devices: " << std::endl;
        for (uint32_t dev = 0; dev < numPhysicalDevices; dev++){
            // Get Properties
            auto physicalDevice = physicalDevices[dev];
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            std::cout << "   Physical Device " << std::hex << dev << ": " << std::endl;
            std::cout << "      API Version: " << std::hex << properties.apiVersion << std::endl;
            std::cout << "      Driver Version: " << std::hex << properties.driverVersion << std::endl;
            std::cout << "      Vendor ID: " << std::hex << properties.vendorID << std::endl;
            std::cout << "      Device ID: " << std::hex << properties.deviceID << std::endl;
            // std::cout << "      Device Type: " << std::endl;
            std::cout << "      Device Name: " << properties.deviceName << std::endl;

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

        // // Enumerate Device Function Pointers
        // VK_DEVICE_FUNCTION(vkCreateDevice);
        // VK_DEVICE_FUNCTION(vkDestroyDevice);
        // VK_DEVICE_FUNCTION(vkDeviceWaitIdle);

        // // Enumerate Devices
        // assert (vkCreateDevice());
    }
}