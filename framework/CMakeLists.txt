add_library("Framework" INTERFACE)

target_include_directories("Framework" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB src "*.hpp" "*.cpp")

add_library("Framework_dummy" STATIC "${src}")
set_target_properties("Framework_dummy" PROPERTIES LINKER_LANGUAGE CXX)