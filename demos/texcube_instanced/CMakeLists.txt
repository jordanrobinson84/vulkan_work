add_executable(texcube_instanced texcube_instanced.cpp)
target_compile_options( texcube_instanced PRIVATE )

add_custom_command(
    TARGET texcube_instanced POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/blkmarbl.png
            ${CMAKE_CURRENT_BINARY_DIR}/blkmarbl.png)
add_custom_command(
    TARGET texcube_instanced POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/frag.spv
            ${CMAKE_CURRENT_BINARY_DIR}/frag.spv)
add_custom_command(
    TARGET texcube_instanced POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/vert.spv
            ${CMAKE_CURRENT_BINARY_DIR}/vert.spv)
if ( WIN32 )
    if(MSVC)
    set_target_properties( texcube_instanced PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties( texcube_instanced PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties( texcube_instanced PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties( texcube_instanced PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties( texcube_instanced PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    target_link_libraries( texcube_instanced VulkanRenderer )
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL Linux )
    target_link_libraries( texcube_instanced m dl ${XCB_LIBS} VulkanRenderer )
endif()