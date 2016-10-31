include(GenerateExportHeader)

add_library( vulkanRenderer STATIC vulkanBuffer.cpp vulkanCommandPool.cpp vulkanDriverInstance.cpp vulkanRenderPass.cpp vulkanSwapchain.cpp vulkanRenderer_Export.h )
#[[generate_export_header( vulkanRenderer 
    BASE_NAME vulkanRenderer
    EXPORT_MACRO_NAME vulkanRenderer_EXPORT
    EXPORT_FILE_NAME vulkanRenderer_Export.h
    STATIC_DEFINE vulkanRenderer_BUILT_AS_STATIC )]]