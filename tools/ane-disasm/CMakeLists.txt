# Copyright 2025. Alexandro Sanchez Bach

cmake_minimum_required(VERSION 3.16)
project(ane-disasm CXX)

# Sources
file(GLOB ANE_DISASM_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(${PROJECT_NAME} ${ANE_DISASM_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ane_object)

# Properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
