include(GenerateExportHeader)

if ( WIN32 )
    add_library( vulkanRenderer STATIC vulkanBuffer.cpp vulkanCommandPool.cpp vulkanDriverInstance.cpp vulkanPipelineState.cpp vulkanRenderPass.cpp vulkanSwapchain.cpp win32Window.cpp)
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL Linux )
    add_library( vulkanRenderer STATIC vulkanBuffer.cpp vulkanCommandPool.cpp vulkanDriverInstance.cpp vulkanPipelineState.cpp vulkanRenderPass.cpp vulkanSwapchain.cpp xcbWindow.cpp)
endif()
#[[generate_export_header( vulkanRenderer 
    BASE_NAME vulkanRenderer
    EXPORT_MACRO_NAME vulkanRenderer_EXPORT
    EXPORT_FILE_NAME vulkanRenderer_Export.h
    STATIC_DEFINE vulkanRenderer_BUILT_AS_STATIC )]]