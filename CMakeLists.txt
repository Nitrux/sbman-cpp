cmake_minimum_required(VERSION 3.16)

set(SBMAN_VERSION 0.0.1)

project(sbman VERSION ${SBMAN_VERSION})

set(QT_MAJOR_VERSION 6)
set(REQUIRED_QT_VERSION 6.9)
set(REQUIRED_KF_VERSION 6.13.0)
set(REQUIRED_MAUIKIT_VERSION 4.0.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project supports Linux only.")
endif()

find_package(ECM ${REQUIRED_KF_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

include(CheckCXXCompilerFlag)
include(ECMSetupVersion)
include(ECMSourceVersionControl)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Qml Quick)
find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS CoreAddons I18n)
find_package(MauiKit4 ${REQUIRED_MAUIKIT_VERSION} REQUIRED)

qt_policy(SET QTP0001 NEW)

if(ECM_SOURCE_UNDER_VERSION_CONTROL)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT "$ENV{GITHUB_HEAD_REF}" STREQUAL "")
        set(GIT_BRANCH "$ENV{GITHUB_HEAD_REF}")
    elseif(NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "")
        set(GIT_BRANCH "$ENV{GITHUB_REF_NAME}")
    endif()
elseif(NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "" AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
    if(NOT "$ENV{GITHUB_HEAD_REF}" STREQUAL "")
        set(GIT_BRANCH "$ENV{GITHUB_HEAD_REF}")
    else()
        set(GIT_BRANCH "$ENV{GITHUB_REF_NAME}")
    endif()
    string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 GIT_COMMIT_HASH)
else()
    set(GIT_BRANCH "Stable")
    set(GIT_COMMIT_HASH "0.0.1")
endif()

if(GIT_COMMIT_HASH STREQUAL "" AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
    string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 GIT_COMMIT_HASH)
endif()

add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")
add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

set(SBMAN_ARCH_FLAGS)
if(NOT CMAKE_CROSSCOMPILING
   AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|X86_64|amd64|AMD64)$")
    check_cxx_compiler_flag("-march=x86-64-v3" SBMAN_COMPILER_SUPPORTS_X86_64_V3)
    if(SBMAN_COMPILER_SUPPORTS_X86_64_V3)
        list(APPEND SBMAN_ARCH_FLAGS -march=x86-64-v3)
    endif()

    check_cxx_compiler_flag("-mtune=generic" SBMAN_COMPILER_SUPPORTS_MTUNE_GENERIC)
    if(SBMAN_COMPILER_SUPPORTS_MTUNE_GENERIC)
        list(APPEND SBMAN_ARCH_FLAGS -mtune=generic)
    endif()
endif()

add_definitions(-DSBMAN_VERSION_STRING="${SBMAN_VERSION}")

add_subdirectory(src)

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE
        @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

qt_finalize_project()
