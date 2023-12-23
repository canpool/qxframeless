cmake_minimum_required(VERSION 3.5)

set(QX_FRAMELESS_VERSION_MAJOR 0)
set(QX_FRAMELESS_VERSION_MINOR 1)
set(QX_FRAMELESS_VERSION_PATCH 0)
set(QX_FRAMELESS_VERSION "${QX_FRAMELESS_VERSION_MAJOR}.${QX_FRAMELESS_VERSION_MINOR}.${QX_FRAMELESS_VERSION_PATCH}")

project(QtFrameless
    LANGUAGES CXX
    DESCRIPTION "A fancy Qt widget frameless helper"
    HOMEPAGE_URL "https://gitee.com/icanpool/qxframeless"
)

if(WIN32)
    set(QX_FRAMELESS_CONFIG_NATIVE ON)
endif()

option(QX_FRAMELESS_CONFIG_BUILD_STATIC "Build QxFrameless as static library" OFF)
option(QX_FRAMELESS_CONFIG_BUILD_DEMOS "Build QxFrameless demos" ON)
option(QX_FRAMELESS_CONFIG_NATIVE, "Use platform native application interface" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(QX_FRAMELESS_PLATFORM "x86")
else()
    set(QX_FRAMELESS_PLATFORM "x64")
endif()

set(QX_FRAMELESS_BIN_DIR "${CMAKE_BINARY_DIR}/${QX_FRAMELESS_PLATFORM}")

add_subdirectory(src)

if(QX_FRAMELESS_CONFIG_BUILD_DEMOS)
    add_subdirectory(demos)
endif()
