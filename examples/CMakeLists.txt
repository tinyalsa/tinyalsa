cmake_minimum_required(VERSION 2.6)


add_executable("tinycap" "tinycap.c")

target_link_libraries("tinycap" "salsa")


add_executable("tinyplay" "tinyplay.c")

target_link_libraries("tinyplay" "salsa")


add_executable("tinypcminfo" "tinypcminfo.c")

target_link_libraries("tinypcminfo" "salsa")


add_executable("tinywavinfo" "tinywavinfo.c")

target_link_libraries("tinywavinfo" "salsa" "m")

