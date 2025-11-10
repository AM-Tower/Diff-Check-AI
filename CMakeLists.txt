#***************************************************************#
# @file CMakeLists.txt
# @brief Build configuration for DiffCheckAI project.
#
# @author Jeffrey Scott Flesher with the help of AI: Copilot
# @version 0.1
# @date 2025-11-07
# @section License MIT
# @section DESCRIPTION
# Configures CMake build for DiffCheckAI, sets C++ standard,
# finds Qt6 components, builds executable, installs, and
# configures deployment and packaging for Windows, macOS, Linux.
# **************************************************************#

cmake_minimum_required(VERSION 3.24)
project(DiffCheckAI VERSION 0.1 LANGUAGES CXX)

# =========================
# Project Maintainer/Contact
# =========================
set(PROJECT_MAINTAINER "Jeffrey Scott Flesher")
set(PROJECT_CONTACT "Occupant.AM.Tower@gmail.com")
# =========================

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific flags --------------------------------
if(MSVC)
    add_compile_options(/Zc:__cplusplus)
    add_compile_options(/permissive-)
endif()

if(MINGW)
    add_compile_definitions(QT_DEFAULT_RHI=opengl)
    add_compile_definitions(QT_QPA_PLATFORM=windows)
    set(QT_FEATURE_shadercache_dxc OFF CACHE BOOL "Disable DXC shader compiler" FORCE)
    set(QT_FEATURE_d3d12 OFF CACHE BOOL "Disable Direct3D 12 backend" FORCE)
    set(QT_FEATURE_direct3d12 OFF CACHE BOOL "Disable Direct3D 12 backend" FORCE)
    set(QT_FEATURE_rhi_direct3d12 OFF CACHE BOOL "Disable D3D12 RHI" FORCE)
    set(QT_OPENGL "desktop" CACHE STRING "Force OpenGL backend" FORCE)
endif()
# ----------------------------------------------------

# Qt6 components
find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Network Concurrent Svg Test)

qt_standard_project_setup()

# Translations
set(TS_FILES
    translations/DiffCheckAI_en.ts
    translations/DiffCheckAI_es.ts
)
qt_add_translation(QM_FILES ${TS_FILES})

# Resources (icons + translations)
qt_add_resources(APP_RESOURCES DiffCheckAI.qrc)

# Executable
qt_add_executable(DiffCheckAI
    src/main.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/CompareEngine.h src/CompareEngine.cpp
    src/Config.h
    src/Settings.h src/Settings.cpp
    ${APP_RESOURCES}
    ${QM_FILES}
)

target_link_libraries(DiffCheckAI PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Concurrent Qt6::Svg)

target_include_directories(DiffCheckAI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Tests
enable_testing()
add_executable(test_settings
    tests/test_settings.cpp
    src/MainWindow.cpp
    src/CompareEngine.cpp
    src/Settings.cpp
    ${APP_RESOURCES}
    # Add any other .cpp files MainWindow depends on
)
target_include_directories(test_settings PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_settings PRIVATE Qt6::Test Qt6::Core Qt6::Gui Qt6::Widgets)
add_test(NAME TestSettingsTab COMMAND test_settings)

# --- Disable Qt ShaderTools DXC probing on MinGW. Still get warnings. ---
if(MINGW)
    set_target_properties(DiffCheckAI PROPERTIES
        QT_DEFAULT_RHI_BACKEND "opengl"
        QT_QPA_PLATFORM_PLUGIN "windows"
    )
endif()
# ----------------------------------------------------
# Package data folder for QtSettings JSon file.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION .)

# Deployment logic
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS "${Qt6_DIR}/../../../bin" ENV PATH)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET DiffCheckAI POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --verbose 0 --no-compiler-runtime "$<TARGET_FILE:DiffCheckAI>"
            COMMENT "Deploying Qt runtime for DiffCheckAI (Windows)"
        )
    endif()
elseif(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE NAMES macdeployqt HINTS "${Qt6_DIR}/../../../bin" ENV PATH)
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET DiffCheckAI POST_BUILD
            COMMAND ${MACDEPLOYQT_EXECUTABLE} "$<TARGET_BUNDLE_DIR:DiffCheckAI>" -verbose=1
            COMMENT "Deploying Qt runtime for DiffCheckAI (macOS)"
        )
    endif()
elseif(UNIX)
    message(STATUS "✅ Linux build – Qt runtime deployment handled by system packages.")
endif()

# Install the executable
install(TARGETS DiffCheckAI DESTINATION bin)

# Packaging with CPack
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "DiffCheckAI")
set(CPACK_PACKAGE_VENDOR "AM-Tower")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Python venv and pip-tools GUI manager")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_EXECUTABLES "DiffCheckAI" "DiffCheckAI")
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "DiffCheckAI")
    set(CPACK_NSIS_PACKAGE_NAME "DiffCheckAI")
    set(CPACK_NSIS_CONTACT "${PROJECT_CONTACT}")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${PROJECT_MAINTAINER}")
endif()
include(CPack)

#************** End of CMakeLists.txt **************************#
