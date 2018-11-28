file(GLOB src "*.hpp" "*.cpp")

add_executable("Test" "${src}")

target_link_libraries("Test" "Framework")

if(MSVC)
	set_target_properties("Test" PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
	set_target_properties("Test" PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
endif(MSVC)