cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0079 NEW) # for line 67

set(PROJECT imguiproc-gl)

enable_language(C CXX)
project(${PROJECT} CXX)

option(USE_STD_FILESYSTEM "Enable std::filesystem use for path and ImGuiFileDialog" ON)
option(USE_NEW_IMGUI_KEY_SYSTEM "Define the symbol IMGUI_DISABLE_OBSOLETE_KEYIO" ON)


include(cmake/3rdparty.cmake)

find_package(OpenCV REQUIRED)

if(MSVC)
    add_definitions(-DMSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
endif ()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (USE_NEW_IMGUI_KEY_SYSTEM)
	add_definitions(-DIMGUI_DISABLE_OBSOLETE_KEYIO)
endif()

file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
file(GLOB CUSTOM_SOURCES ${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h)
file(GLOB RES_SOURCES 
		${CMAKE_SOURCE_DIR}/CustomFont.cpp 
		${CMAKE_SOURCE_DIR}/CustomFont.h)
file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h)
file(GLOB IMGPROC_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
source_group(main FILES ${MAIN_SOURCES})
source_group(src\\Custom FILES ${CUSTOM_SOURCES})
source_group(src\\Res FILES ${RES_SOURCES})
source_group(src\\ImGuiImpl FILES ${IMGUIIMPL_SOURCES})

if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

include_directories(
		${PLOG_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${OPENGL_INCLUDE_DIR}
		${GLFW_INCLUDE_DIR}
		${GLAD_INCLUDE_DIR}
		${IMGUI_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}
		${OpenCV_INCLUDE_DIRS}
)

if (USE_TEST)
	add_definitions(-DNEED_TO_BE_PUBLIC_FOR_TESTS)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/ImGuiFileDialog)

target_link_libraries(ImGuiFileDialog PRIVATE
		${IMGUI_LIBRARIES}
)

add_executable(${PROJECT}
		MACOSX_BUNDLE
		##${ICON}
		${RES_SOURCES}
		${MAIN_SOURCES}
		${CUSTOM_SOURCES}
		${IMGUIIMPL_SOURCES}
		${IMGPROC_SOURCES}
)

if(UNIX)
    target_compile_options(${PROJECT} PUBLIC "-Wno-unknown-pragmas")
endif()

if (APPLE)
	set_target_properties(${PROJECT}
		PROPERTIES
		OUTPUT_NAME "${PROJECT}_macos"
		MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT}"
		MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT}"
		FOLDER "${PROJECT}"
		##RESOURCE icon.icns
		MACOSX_BUNDLE_SHORT_VERSION_STRING ""
		MACOSX_BUNDLE_LONG_VERSION_STRING ""
		##MACOSX_BUNDLE_ICON_FILE icon.icns
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_win32")
elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_unix")
endif()

if (USE_STD_FILESYSTEM)
	add_definitions(-DUSE_STD_FILESYSTEM)
	set(CMAKE_CXX_STANDARD 17)
	set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 17)
	set_target_properties(ImGuiFileDialog PROPERTIES CXX_STANDARD 17)
	set_target_properties(imgui PROPERTIES CXX_STANDARD 17)
	message(STATUS "USE C++17 and std::filesystem")
else()
	set(CMAKE_CXX_STANDARD 11)
	set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 11)
	set_target_properties(ImGuiFileDialog PROPERTIES CXX_STANDARD 11)
	set_target_properties(imgui PROPERTIES CXX_STANDARD 11)
	message(STATUS "USE C++11 and DIRENT")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
	ImGuiFileDialog
		${OpenCV_LIBS}
)

set_target_properties(ImGuiFileDialog PROPERTIES FOLDER Lib)

target_include_directories(ImGuiFileDialog PRIVATE
	${CMAKE_SOURCE_DIR}
)