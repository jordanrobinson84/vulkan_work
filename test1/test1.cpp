#include <iostream>
#include <cassert>
#include "vulkanDriverInstance.h"
#include "vulkanBuffer.h"

int main(){
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

    // Set buffer data
    float vertexBufferData[9];
    vertexBufferData[0] = 0.2; // x[0]
    vertexBufferData[1] = 0.2; // y[0]
    vertexBufferData[2] = 0.0; // z[0]
    vertexBufferData[3] = 0.2; // x[1]
    vertexBufferData[4] = 0.8; // y[1]
    vertexBufferData[5] = 0.0; // z[1]
    vertexBufferData[6] = 0.8; // x[2]
    vertexBufferData[7] = 0.5; // y[2]
    vertexBufferData[8] = 0.0; // z[2]

    VulkanBuffer vertexBuffer(deviceContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBufferData, sizeof(float) * 9, false);

    return true;
}
