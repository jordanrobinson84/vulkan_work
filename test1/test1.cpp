#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include <iostream>
#include "vulkanDriverInstance.h"

int main(){
    // void *vulkanLibrary = nullptr;
    // vulkanLibrary = dlopen( "libvulkan.so", RTLD_NOW);
    VulkanDriverInstance instance("Linux");

    if(instance.loader == nullptr){
        std::cout << "Vulkan library not found!" << std::endl;
        return false;
    }
    std::cout << "Vulkan library found!" << std::endl;
    if (instance.vkGetInstanceProcAddr != nullptr){
        std::cout << "vkGetInstanceProcAddr function found!" << std::endl;
    }
    if (instance.vkCreateInstance != nullptr){
        std::cout << "vkCreateInstance function found!" << std::endl;
    }
    return true;
}
