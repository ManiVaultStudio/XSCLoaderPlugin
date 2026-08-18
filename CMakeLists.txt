cmake_minimum_required(VERSION 3.22)

# -----------------------------------------------------------------------------
# XSCLoaderPlugin
# -----------------------------------------------------------------------------
PROJECT("XSCLoaderPlugin")

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
if(NOT DEFINED MV_CSCTD_INSTALL_DIR) 
    set(MV_CSCTD_INSTALL_DIR ${ManiVault_INSTALL_DIR} CACHE PATH "Directory where tree data is installed")
endif()

if(NOT DEFINED MV_CSCTMD_INSTALL_DIR) 
    set(MV_CSCTMD_INSTALL_DIR ${ManiVault_INSTALL_DIR} CACHE PATH "Directory where tree meta data is installed")
endif()

#find_package(OpenSSL REQUIRED)
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets Concurrent REQUIRED)

find_package(ManiVault COMPONENTS Core PointData ClusterData CONFIG QUIET)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
# Define the plugin sources
set(XSCLOADER_SOURCES
    src/XSCLoaderPlugin.h
    src/XSCLoaderPlugin.cpp
    src/XSCLoaderPlugin.json
	src/InputDialogCSV.h
    src/InputDialogCSV.cpp
	src/InputDialogJSON.h
    src/InputDialogJSON.cpp
)

set(PLUGIN_MOC_HEADERS
	src/XSCLoaderPlugin.h
)

source_group( Plugin FILES ${XSCLOADER_SOURCES})
source_group( LibUtils FILES ${XSCLOADER_LibUtils})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
# Create dynamic library for the plugin
add_library(${PROJECT_NAME} SHARED ${XSCLOADER_SOURCES} ${XSCLOADER_LibUtils})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault headers, including system data plugins
target_include_directories(${PROJECT_NAME} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${MV_CSCTD_INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PROJECT_NAME} PRIVATE "${MV_CSCTMD_INSTALL_DIR}/$<CONFIGURATION>/include/")
#target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/curl/include/")


# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# Link to Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Concurrent )


# Allow override of plugin library paths from the build system
set(XSCTREEDATA_LINK_LIBRARY "${XSCTREEDATA_LINK_LIBRARY}" CACHE STRING "Path to XSCTreeData library")
if(NOT XSCTREEDATA_LINK_LIBRARY)
    set(CSCTDPLUGIN_LINK_PATH "${MV_CSCTD_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
    set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)
    set(XSCTREEDATA_LINK_LIBRARY "${CSCTDPLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}XSCTreeData${MV_LINK_SUFFIX}")
endif()

set(XSCTREEMETADATA_LINK_LIBRARY "${XSCTREEMETADATA_LINK_LIBRARY}" CACHE STRING "Path to XSCTreeMetaData library")
if(NOT XSCTREEMETADATA_LINK_LIBRARY)
    set(CSCTMDPLUGIN_LINK_PATH "${MV_CSCTMD_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
    set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)
    set(XSCTREEMETADATA_LINK_LIBRARY "${CSCTMDPLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}XSCTreeMetaData${MV_LINK_SUFFIX}")
endif()

# Link to ManiVault and data plugins
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::ClusterData)

target_link_libraries(${PROJECT_NAME} PRIVATE "${XSCTREEDATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT_NAME} PRIVATE "${XSCTREEMETADATA_LINK_LIBRARY}")

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
# Install the shared plugin libary to the "Plugins" folder in the ManiVault install directory
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug/ManiVault\ Studio.exe,${ManiVault_INSTALL_DIR}/release/ManiVault\ Studio.exe>)
endif()
