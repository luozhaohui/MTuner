cmake_minimum_required(VERSION 2.6)

project(rmem)

include(${TN_BUILDKIT_DIR}/lib/cmake/link_tools.cmake)

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/src/libs)
set(RBASE_DIR ${LIBS_DIR}/rbase)
set(RDEBUG_DIR ${LIBS_DIR}/rdebug)
set(RMEM_DIR ${LIBS_DIR}/rmem)
set(RQT_DIR ${LIBS_DIR}/rqt)

set(MTUNER_DIR ${CMAKE_SOURCE_DIR}/src/MTuner)
set(MTUNER_CMD_DIR ${CMAKE_SOURCE_DIR}/src/tools/cmdline/MTunerCmd)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/3rd)

# Enable build of rmem target according to the platform
set(BUILD_RMEM_TARGET OFF)
if ((DEFINED TN_PLATFORM_WIN64)
    OR (DEFINED TN_PLATFORM_OSX)
    OR (DEFINED TN_PLATFORM_ANDROID)
    OR (DEFINED TN_PLATFORM_LINUX_UBUNTU)
)
    set(BUILD_RMEM_TARGET ON)
endif()

set(BUILD_RMEM_UI_TARGET OFF)

if (TN_USE_TCMALLOC)
    MESSAGE(STATUS "tc_malloc is on")
else()
    MESSAGE(STATUS "tc_malloc is off")
endif()

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)


if (CMAKE_CONFIGURATION_TYPES)
    foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOLOWER ${cfg} lower_cfg)
        if (lower_cfg MATCHES "debug")
            add_definitions(-DRTM_DEBUG_BUILD)
            add_definitions(-D_DEBUG)
            add_definitions(-DDEBUG)
        else()
            add_definitions(-DRTM_RELEASE_BUILD)
            add_definitions(-DNDEBUG)
        endif()
    endforeach()
elseif (CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE MATCHES "debug")
        add_definitions(-DRTM_DEBUG_BUILD)
        add_definitions(-D_DEBUG)
        add_definitions(-DDEBUG)
    else()
        add_definitions(-DRTM_RELEASE_BUILD)
        add_definitions(-DNDEBUG)
    endif()
endif()

if (DEFINED TN_PLATFORM_WIN64)
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32)
    add_definitions(-D_HAS_EXCEPTIONS=0)
    add_definitions(-D_SCL_SECURE=0)
    add_definitions(-D_SECURE_SCL=0)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_definitions(-DRTM_WIN64)
    add_definitions(-DRTM_WINDOWS)
    add_definitions(-D_WIN64)
endif()

message(STATUS "PROJECT_SOURCE_DIR:${PROJECT_SOURCE_DIR}")

if (BUILD_RMEM_TARGET)
    include_directories(${TN_MODULES_DIR}/include)

    include_directories(${RMEM_DIR}/inc)
    include_directories(${RMEM_DIR}/src)
    include_directories(${RMEM_DIR}/3rd)

    include_directories(${MTUNER_DIR}/src)
    include_directories(${MTUNER_CMD_DIR}/src)

    include_directories(${LIBS_DIR})

    # 1, build the lib for rmem
    aux_source_directory(${RMEM_DIR}/src rmem_src)
    add_library(${PROJECT_NAME} STATIC ${rmem_src})
    if (TN_USE_TCMALLOC)
        target_link_libraries(${PROJECT_NAME} PUBLIC tcmalloc_minimal_debug)
    endif()
    target_link_libraries(${PROJECT_NAME} PUBLIC lz4)

    tn_format_code(${PROJECT_NAME} ${rmem_src})

    # 1.1, build rmem samples
    if (DEFINED TN_PLATFORM_OSX OR DEFINED TN_PLATFORM_WIN64 OR DEFINED TN_PLATFORM_LINUX_UBUNTU)
        add_subdirectory(${RMEM_DIR}/samples/)
    endif()

    # 2, build the rbase lib.
    set(MTUNER_RBASE rbase)
    aux_source_directory(${RBASE_DIR}/src rbase_src)
    add_library(${MTUNER_RBASE} STATIC ${rbase_src})
    #target_compile_definitions(${MTUNER_RBASE} PUBLIC RBASE_NAMESPACE)

    tn_format_code(${MTUNER_RBASE} ${rbase_src})

    # 3, build the rdebug lib
    set(MTUNER_RDEBUG rdebug)
    aux_source_directory(${RDEBUG_DIR}/src rdebug_src)
    add_library(${MTUNER_RDEBUG} STATIC ${rdebug_src})

    target_link_libraries(${MTUNER_RDEBUG} rbase)

    if (DEFINED TN_PLATFORM_WIN64)
        link_directories(${THIRD_PARTY_DIR}/DIA/lib/X64)
        target_link_libraries(${MTUNER_RDEBUG} diaguids)
    endif()

    tn_format_code(${MTUNER_RDEBUG} ${rdebug_src})

    # 4, build mtuner cmdline program
    set(MTUNER_CMD mtuner_cmd)
    aux_source_directory(${MTUNER_CMD_DIR}/src mtuner_cmd_src)
    add_executable(${MTUNER_CMD} ${mtuner_cmd_src})
    target_link_libraries(${MTUNER_CMD} rdebug)

    tn_format_code(${MTUNER_CMD} ${mtuner_cmd_src})

    # 5, build mtuner QT UI program
    if (BUILD_RMEM_UI_TARGET)
        include(${TN_BUILDKIT_DIR}/lib/cmake/findQT.cmake)

        set(MTUNER mtuner)
        aux_source_directory(${MTUNER_DIR}/src mtuner_src)
        add_executable(${MTUNER} ${mtuner_src})
        target_link_libraries(${MTUNER} rdebug lz4 boost_system boost_filesystem)

        tn_format_code(${MTUNER} ${mtuner_src})
    endif()
endif()

####################################clang-format####################################

tn_format_code_all()