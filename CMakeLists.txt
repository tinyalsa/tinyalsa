cmake_minimum_required(VERSION 3.1)

execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/version.sh -s print
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE TINYALSA_VERSION)

project("TinyALSA" VERSION ${TINYALSA_VERSION} LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(TINYALSA_USES_PLUGINS "Whether or not to build with plugin support" ON)
option(TINYALSA_BUILD_EXAMPLES "Build examples" ON)
option(TINYALSA_BUILD_UTILS "Build utility tools" ON)

# Library
add_library("tinyalsa"
    "src/pcm.c"
    "src/pcm_hw.c"
    "src/pcm_plugin.c"
    "src/snd_card_plugin.c"
    "src/mixer.c"
    "src/mixer_hw.c"
    "src/mixer_plugin.c")

set_property(TARGET "tinyalsa" PROPERTY PUBLIC_HEADER
    "include/tinyalsa/attributes.h"
    "include/tinyalsa/version.h"
    "include/tinyalsa/asoundlib.h"
    "include/tinyalsa/pcm.h"
    "include/tinyalsa/plugin.h"
    "include/tinyalsa/mixer.h")

set_target_properties("tinyalsa" PROPERTIES
    VERSION ${TinyALSA_VERSION}
    SOVERSION ${TinyALSA_VERSION_MAJOR})

target_include_directories("tinyalsa" PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_definitions("tinyalsa" PRIVATE
    $<$<BOOL:${TINYALSA_USES_PLUGINS}>:TINYALSA_USES_PLUGINS>
    PUBLIC _POSIX_C_SOURCE=200809L)
target_link_libraries("tinyalsa" PUBLIC ${CMAKE_DL_LIBS})

# Examples
if(TINYALSA_BUILD_EXAMPLES)
    set(TINYALSA_EXAMPLES pcm-readi pcm-writei)
else()
    set(TINYALSA_EXAMPLES)
endif()

foreach(EXAMPLE IN LISTS TINYALSA_EXAMPLES)
    add_executable("${EXAMPLE}" "examples/${EXAMPLE}.c")
    target_link_libraries("${EXAMPLE}" PRIVATE "tinyalsa")
endforeach()

# Utilities
if(TINYALSA_BUILD_UTILS)
    set(TINYALSA_UTILS tinyplay tinycap tinypcminfo tinymix tinywavinfo)
else()
    set(TINYALSA_UTILS)
endif()

foreach(UTIL IN LISTS TINYALSA_UTILS)
    add_executable("${UTIL}" "utils/${UTIL}.c")
    target_link_libraries("${UTIL}" PRIVATE "tinyalsa")
endforeach()

if(TINYALSA_BUILD_UTILS)
    target_link_libraries("tinywavinfo" PRIVATE m)
endif()

# Add C warning flags
include(CheckCCompilerFlag)
foreach(FLAG IN ITEMS -Wall -Wextra -Wpedantic -Werror -Wfatal-errors)
    string(TOUPPER "HAVE${FLAG}" HAVE_VAR)
    string(REPLACE "-" "_" HAVE_VAR "${HAVE_VAR}")
    check_c_compiler_flag("${FLAG}" "${HAVE_VAR}")
    if("${${HAVE_VAR}}")
        target_compile_options("tinyalsa" PRIVATE "${FLAG}")
        foreach(UTIL IN LISTS TINYALSA_UTILS)
            target_compile_options("${UTIL}" PRIVATE "${FLAG}")
        endforeach()
    endif()
endforeach()

# Install
include(GNUInstallDirs)
install(TARGETS "tinyalsa" ${TINYALSA_UTILS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinyalsa)
