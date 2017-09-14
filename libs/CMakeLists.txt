include(GenerateExportHeader)

if ( WIN32 )
    add_library( VulkanRenderer STATIC VulkanBuffer.cpp VulkanCommandPool.cpp VulkanDriverInstance.cpp VulkanPipelineState.cpp VulkanRenderPass.cpp VulkanSwapchain.cpp Win32Window.cpp)
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL Linux )
    add_library( VulkanRenderer STATIC VulkanBuffer.cpp VulkanCommandPool.cpp VulkanDriverInstance.cpp VulkanPipelineState.cpp VulkanRenderPass.cpp VulkanSwapchain.cpp XCBWindow.cpp)
endif()
#[[generate_export_header( VulkanRenderer 
    BASE_NAME VulkanRenderer
    EXPORT_MACRO_NAME VulkanRenderer_EXPORT
    EXPORT_FILE_NAME VulkanRenderer_Export.h
    STATIC_DEFINE VulkanRenderer_BUILT_AS_STATIC )]]