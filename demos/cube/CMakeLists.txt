add_executable(cube cube.cpp)
target_compile_options( cube PRIVATE )

add_custom_command(
    TARGET cube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/frag.spv
            ${CMAKE_CURRENT_BINARY_DIR}/frag.spv)
add_custom_command(
    TARGET cube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/vert.spv
            ${CMAKE_CURRENT_BINARY_DIR}/vert.spv)
if ( WIN32 )
    if(MSVC)
    set_target_properties( cube PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties( cube PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties( cube PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties( cube PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties( cube PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    target_link_libraries( cube VulkanRenderer )
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL Linux )
    target_link_libraries( cube m dl ${XCB_LIBS} VulkanRenderer )
endif()