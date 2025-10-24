cmake_minimum_required(VERSION 3.26)
project(DataObjectEditor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
# Key Configuration Flags (must set BEFORE adding subdirectories)
# ========================================
set(CEE_USE_QT6 ON CACHE BOOL "Use Qt6" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(CAF_EXCLUDE_CVF OFF CACHE BOOL "Include minimal CVF support" FORCE)

# Disable ResInsight-specific features we don't need
set(RESINSIGHT_ENABLE_GRPC OFF CACHE BOOL "Disable gRPC" FORCE)
set(RESINSIGHT_USE_ODB_API OFF CACHE BOOL "Disable ODB" FORCE)
set(RESINSIGHT_ENABLE_UNITY_BUILD OFF CACHE BOOL "Disable unity build" FORCE)
set(CEE_BUILD_UNIT_TESTS OFF CACHE BOOL "Skip VizFwk tests" FORCE)
set(CEE_BUILD_TEST_APPS OFF CACHE BOOL "Skip VizFwk test apps" FORCE)
set(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "Disable warnings as errors" FORCE)
set(RESINSIGHT_INCLUDE_APPFWK_TESTS OFF CACHE BOOL "Skip AppFwk tests" FORCE)

# ========================================
# vcpkg Integration
# ========================================
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/external/ResInsight/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/external/ResInsight/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# ========================================
# Find Dependencies
# ========================================
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Svg)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg)
qt_standard_project_setup()

find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem regex system)

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
# Minimal VizFwk (LibCore only for CVF types)
# ========================================
add_subdirectory(external/ResInsight/Fwk/VizFwk/LibCore)

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

# Animation Control
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafAnimControl)

# Data Loader
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafDataLoader)

# CVF Integration (requires LibCore)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafPdmCvf)

# Tensor Support (requires LibCore)
add_subdirectory(external/ResInsight/Fwk/AppFwk/cafTensor)

set(APP_FWK_LIBRARIES
    cafCommandFeatures
    cafCommand
    cafUserInterface
    cafPdmCvf
    cafPdmScripting
    cafProjectDataModel
    cafPdmXml
    cafPdmUiCore
    cafPdmCore
    cafAnimControl
    cafDataLoader
    cafTensor
    LibCore
)

# Set folder organization for IDE
set_property(TARGET ${APP_FWK_LIBRARIES} PROPERTY FOLDER "AppFwk")
set_property(TARGET custom-opm-common PROPERTY FOLDER "ThirdParty")

# ========================================
# Application
# ========================================
add_subdirectory(src)
