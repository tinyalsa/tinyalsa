cmake_minimum_required(VERSION 2.6)

project("Salsa")

include_directories("include")

if (CMAKE_COMPILER_IS_GNUCC)
	set (CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wfatal-errors -std=gnu99")
endif (CMAKE_COMPILER_IS_GNUCC)

set (SALSA_SOURCES
	"pcm.c"
	"pcm-2.c"
	"mixer.c")

add_library("salsa" ${SALSA_SOURCES})

add_subdirectory("examples")

