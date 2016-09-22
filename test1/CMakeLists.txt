add_executable(test1 test1.cpp)
target_compile_options( test1 PRIVATE )
target_link_libraries( test1 m dl ${XCB_LIBS} vulkanRenderer )