cmake_minimum_required(VERSION 3.0.2)

project("TinyALSA" C)

set (HDRS
    "include/tinyalsa/attributes.h"
    "include/tinyalsa/version.h"
    "include/tinyalsa/asoundlib.h"
    "include/tinyalsa/pcm.h"
    "include/tinyalsa/pcm_plugin.h"
    "include/tinyalsa/mixer.h"
    "include/tinyalsa/mixer_plugin.h")

set (SRCS
    "src/pcm.c"
    "src/pcm_hw.c"
    "src/pcm_plugin.c"
    "src/snd_utils.c"
    "src/mixer.c"
    "src/mixer_hw.c"
    "src/mixer_plugin.c")

add_library("tinyalsa" ${HDRS} ${SRCS})
target_compile_options("tinyalsa" PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
target_include_directories("tinyalsa" PRIVATE "include")

macro(ADD_EXAMPLE EXAMPLE)
    add_executable(${EXAMPLE} ${ARGN})
    target_link_libraries(${EXAMPLE} "tinyalsa")
    target_link_libraries(${EXAMPLE} "dl")
    target_include_directories(${EXAMPLE} PRIVATE "include")
endmacro(ADD_EXAMPLE EXAMPLE)

add_example("pcm-readi" "examples/pcm-readi.c")
add_example("pcm-writei" "examples/pcm-writei.c")

macro(ADD_UTIL UTIL)
    add_executable(${UTIL} ${ARGN})
    target_link_libraries(${UTIL} "tinyalsa")
    target_link_libraries(${UTIL} "dl")
    target_compile_options(${UTIL} PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
    target_include_directories(${UTIL} PRIVATE "include")
endmacro(ADD_UTIL UTIL)

add_util("tinyplay" "utils/tinyplay.c")
add_util("tinycap" "utils/tinycap.c")
add_util("tinypcminfo" "utils/tinypcminfo.c")
add_util("tinymix" "utils/tinymix.c")

install(FILES ${HDRS}
    DESTINATION "include/tinyalsa")

install(TARGETS "tinyalsa"
                "tinyplay"
                "tinycap"
                "tinymix"
                "tinypcminfo"
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

