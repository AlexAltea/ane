# Copyright 2025. Alexandro Sanchez Bach.

cmake_minimum_required(VERSION 3.16)
project(ane-tests CXX)

# Dependencies
include(GoogleTest)
find_package(GTest CONFIG REQUIRED)

# Sources
file(GLOB_RECURSE ANE_TESTS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME} ${ANE_TESTS_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ane_object)
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::gtest)
gtest_discover_tests(${PROJECT_NAME} DISCOVERY_TIMEOUT 30)

# Properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)

# Data
file(GLOB_RECURSE HWX_FILES "${ANE_DIR_TESTS}/*.hwx")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/data
    COMMAND ${CMAKE_COMMAND} -E copy ${HWX_FILES} ${CMAKE_CURRENT_BINARY_DIR}/data)
