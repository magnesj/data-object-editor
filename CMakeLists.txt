cmake_minimum_required(VERSION 3.26)
project(DataObjectEditor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable multi-processor compilation and Unicode support
if(MSVC)
    add_compile_options(/MP /utf-8)
endif()

# ========================================
# Key Configuration Flags (must set BEFORE adding subdirectories)
# ========================================
set(CAF_EXCLUDE_CVF ON CACHE BOOL "Include minimal CVF support" FORCE)

# ========================================
# vcpkg Integration
# ========================================
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# ========================================
# Find Dependencies
# ========================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)
qt_standard_project_setup()

# ========================================
# opm-common Configuration
# ========================================
set(CREATE_OPM_COMMON_KEYWORDS OFF CACHE BOOL "Use pre-generated keywords" FORCE)
set(ENABLE_ECL_INPUT ON CACHE BOOL "" FORCE)
set(ENABLE_ECL_OUTPUT ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPM_ENABLE_PYTHON OFF CACHE BOOL "" FORCE)
set(OPM_ENABLE_EMBEDDED_PYTHON OFF CACHE BOOL "" FORCE)
set(OPM_ENABLE_DUNE OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_MOCKSIM OFF CACHE BOOL "" FORCE)

add_subdirectory(external/ResInsight/ThirdParty/custom-opm-common)

# ========================================
# AppFwk Components
# ========================================

# PDM Core System
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafProjectDataModel)

# Scripting (with CVF support)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafPdmScripting)

# UI Framework
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafUserInterface)

# Command System
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafCommand)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafCommandFeatures)

set(APP_FWK_LIBRARIES
    cafCommandFeatures
    cafCommand
    cafUserInterface
    cafPdmScripting
    cafProjectDataModel
    cafPdmXml
    cafPdmUiCore
    cafPdmCore
)

# ========================================
# Application
# ========================================
add_subdirectory(src)
