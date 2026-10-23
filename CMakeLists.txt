cmake_minimum_required(VERSION 3.25)

project(TacticalAISandbox LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#! ! ! ! ! ! !
#set this to ON instead of OFF to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#delete the out folder after changing if visual studio doesn recognize the change
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

if(MSVC) 
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
else()
	add_compile_options(-mavx2 -mfma) # make sure SIMD optimizations take place, but on linux and other platforms
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies as static libraries" FORCE)

# 1. Fetch GLFW automatically from GitHub
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # Use the specific version you need
)

# 2. Prevent GLFW & GLM from building its internal tests and documentation
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only" FORCE)
set(GLM_TEST_ENABLE OFF CACHE INTERNAL "Disable GLM test bench")

FetchContent_MakeAvailable(glfw)

# 3. Fetch Jolt Physics. Its CMakeLists lives in the Build/ subfolder.
FetchContent_Declare(
    JoltPhysics
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    GIT_TAG        v5.2.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  Build
)

# Library only: no samples, tests or viewer
set(TARGET_UNIT_TESTS OFF CACHE BOOL "Jolt lib only" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "Jolt lib only" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "Jolt lib only" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "Jolt lib only" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "Jolt lib only" FORCE)
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "Don't treat Jolt warnings as errors" FORCE)
# Match our static runtime (/MT, /MTd) and AVX2 settings above
set(USE_STATIC_MSVC_RUNTIME_LIBRARY ON CACHE BOOL "Jolt static CRT" FORCE)
set(USE_AVX2 ON CACHE BOOL "Jolt AVX2" FORCE)

FetchContent_MakeAvailable(JoltPhysics)

# 4. Fetch Dear ImGui. It has no CMakeLists, so we build it as a static library ourselves.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(
	"${CMAKE_PROJECT_NAME}" 
	thirdParty/glad/src/glad.c
	thirdParty/stb/stb_image.cpp
)

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	if(PRODUCTION_BUILD)
		#YOU CAN REMOVE THE CONSOLE WITH THIS LINE!
		set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	endif()

elseif(MINGW) # Windows / MinGW GCC/Clang
	if(PRODUCTION_BUILD)
		# Remove console on Windows under MinGW
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE
			-mwindows
		)
	endif()

elseif(APPLE OR UNIX) # macOS / Linux
	if(PRODUCTION_BUILD)
		target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -s)
	endif()
endif()


target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/gameLayer")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/platform")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC thirdParty/glad/include)
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC thirdParty/stb)
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC thirdParty/glm-0.9.8.5/glm)

# Link GLFW, Jolt and ImGui statically
# (Jolt exports its JPH_* compile definitions through the target, so our code is
#  compiled with the same settings as the library; a mismatch causes subtle crashes)
target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
		glfw
		Jolt
		imgui
)