cmake_minimum_required ( VERSION 2.8 )
project ( VULKAN_WORK )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall" )

message("CMake System Name: ${CMAKE_SYSTEM_NAME}")

include_directories( 
    "include"
    "$ENV{VK_SDK_PATH}/Include" )

if ( WIN32 )
endif()
if ( CMAKE_SYSTEM_NAME STREQUAL Linux )
    set( XCB_LIBS xcb xcb-keysyms xcb-randr )
endif()
add_subdirectory( libs )
add_subdirectory( test1 )

