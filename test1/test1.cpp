#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include <iostream>
#include <cassert>
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

    // Set up instance variables and functions
    VulkanDevice * deviceContext = nullptr;
    deviceContext = instance.getDevice(0);
    if (deviceContext == nullptr){
        std::cout << "Could not create a Vulkan Device!" << std:: endl;
        return false;
    }

    VkBuffer *vertexBuffer = new VkBuffer();
    VkBufferView *vertexBufferView = new VkBufferView();
    VkMemoryRequirements *vertexBufferMemoryRequirements = new VkMemoryRequirements();

    // Buffer Creation Info
    VkBufferCreateInfo vertexBufferInfo;
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.pNext = nullptr;
    vertexBufferInfo.flags = 0;
    vertexBufferInfo.size = 36;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertexBufferInfo.queueFamilyIndexCount = 0; // Not sharing
    vertexBufferInfo.pQueueFamilyIndices = nullptr;

    assert(deviceContext->vkCreateBuffer(deviceContext->device, &vertexBufferInfo, nullptr, vertexBuffer) == VK_SUCCESS);

    // Buffer View Creation Info
    VkBufferViewCreateInfo vertexBufferViewInfo;
    vertexBufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    vertexBufferViewInfo.pNext = nullptr;
    vertexBufferViewInfo.flags = 0;
    vertexBufferViewInfo.buffer = *vertexBuffer;
    vertexBufferViewInfo.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexBufferViewInfo.offset = 0;
    vertexBufferViewInfo.range = VK_WHOLE_SIZE; // Use the entire buffer
    assert(deviceContext->vkCreateBufferView(deviceContext->device, &vertexBufferViewInfo, nullptr, vertexBufferView) == VK_SUCCESS);

    // Get Memory Requirements
    deviceContext->vkGetBufferMemoryRequirements(deviceContext->device, *vertexBuffer, vertexBufferMemoryRequirements);

    // Allocate Memory
    VkDeviceMemory *vertexBufferMemory = new VkDeviceMemory();
    VkMemoryAllocateInfo vertexBufferAllocateInfo;
    vertexBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vertexBufferAllocateInfo.pNext = nullptr;
    vertexBufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements->size;
    vertexBufferAllocateInfo.memoryTypeIndex = 0;
    assert( deviceContext->vkAllocateMemory(deviceContext->device, &vertexBufferAllocateInfo, nullptr, vertexBufferMemory) == VK_SUCCESS);

    // Bind Memory for buffer
    assert( deviceContext->vkBindBufferMemory(deviceContext->device, *vertexBuffer, *vertexBufferMemory, 0) == VK_SUCCESS);

    return true;
}
