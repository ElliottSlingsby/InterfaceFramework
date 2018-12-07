cmake_minimum_required(VERSION 3.4)

project("stb" C CXX)

add_library("stb" INTERFACE)

download_file("stb_image.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
download_file("stb_truetype.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h")
download_file("stb_rect_pack.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h")

target_include_directories("stb" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions("stb" INTERFACE STB_IMAGE_IMPLEMENTATION=1)
target_compile_definitions("stb" INTERFACE STB_TRUETYPE_IMPLEMENTATION=1)
target_compile_definitions("stb" INTERFACE STB_RECT_PACK_IMPLEMENTATION=1)

file(GLOB_RECURSE src "*.h")

add_library("stb_dummy" STATIC ${src})
set_target_properties("stb_dummy" PROPERTIES LINKER_LANGUAGE CXX)