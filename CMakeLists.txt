# Copyright 2025. Alexandro Sanchez Bach

cmake_minimum_required(VERSION 3.16)

# Build options
option(BUILD_LIB              "Build the main libane library" ON)
option(BUILD_TESTS            "Build tests" ON)
option(BUILD_TOOLS            "Build tools" ON)
option(BUILD_BINDINGS         "Build bindings for all languages" OFF)

# Build bindings
option(BUILD_BINDINGS_PYTHON  "Build bindings for Python" ${BUILD_BINDINGS})

# Directories
set(ANE_DIR_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(ANE_DIR_BINDINGS "${ANE_DIR_ROOT}/bindings")
set(ANE_DIR_DRIVER   "${ANE_DIR_ROOT}/driver")
set(ANE_DIR_SOURCES  "${ANE_DIR_ROOT}/src")
set(ANE_DIR_TESTS    "${ANE_DIR_ROOT}/tests")
set(ANE_DIR_TOOLS    "${ANE_DIR_ROOT}/tools")

# Project
project(ane C CXX)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

# Sources
macro(ANE_FILES_APPEND)
    file(GLOB FILES_APPEND CONFIGURE_DEPENDS ${ARGV1})
    list(APPEND ${ARGV0} ${FILES_APPEND})
endmacro()
macro(ANE_FOLDER_APPEND)
    ANE_FILES_APPEND(ANE_HEADERS "${ARGV0}/*.h")
    ANE_FILES_APPEND(ANE_SOURCES "${ARGV0}/*.c")
    ANE_FILES_APPEND(ANE_SOURCES "${ARGV0}/*.cpp")
endmacro()

set(ANE_HEADERS)
set(ANE_SOURCES)
ANE_FOLDER_APPEND(${ANE_DIR_SOURCES}/libane)

# Objects
add_library(ane_object OBJECT ${ANE_SOURCES})
target_include_directories(ane_object PRIVATE ${LIBDRM_INCLUDE_DIRS})
target_include_directories(ane_object PUBLIC ${ANE_DIR_SOURCES} ${ANE_DIR_DRIVER}/src/uapi/drm)
target_link_libraries(ane_object ${LIBDRM_LIBRARIES})
set_target_properties(ane_object PROPERTIES CXX_STANDARD 20)
set_target_properties(ane_object PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(ane_object PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(ane_object PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_include_directories(ane_object PUBLIC ${ANE_DIR_SOURCES}/libane)

# Libraries
if (BUILD_LIB)
    add_library(ane $<TARGET_OBJECTS:ane_object>)
endif()

# Tests
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Tools
if (BUILD_TOOLS)
    add_subdirectory(tools/ane-disasm)
endif()

# Installation
if (INSTALL_LIB)
    include(GNUInstallDirs)
    install(TARGETS ane
        EXPORT aneTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    install(DIRECTORY "${ANE_DIR_SOURCES}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h")
endif()
