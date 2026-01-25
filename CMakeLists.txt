cmake_minimum_required(VERSION "3.19.2")

option(PRODUCTION_BUILD "Build in production mode" OFF)

project(VoxelGame)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-dynamic")

#set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")

#set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set(CMAKE_C_FLAGS_RELEASE "-O3")

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fmt REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/vkbootstrap/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui*.cpp")
#file(GLOB SIMDJSON_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/external/simdjson/simdjson.cpp")
#if(NOT ENABLE_FEATURE_B)
#    file(GLOB_RECURSE FEATURE_B_FILES CONFIGURE_DEPENDS src/featureB/*.cpp src/featureB/*.h)
#    list(REMOVE_ITEM SRC_FILES ${FEATURE_B_FILES})
#endif()

set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET "X86_64")
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 1)
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_SIMD 1)
set(WAMR_ROOT_DIR external/wamr)

include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
#add_library(simdjson ${SIMDJSON_SOURCES})

add_executable(VoxelGame "${MY_SOURCES}")

#if(MSVC)
#  target_compile_options(VoxelGame PRIVATE /W4 /WX)
#else()
#  target_compile_options(VoxelGame PRIVATE -Wall -Wextra -Wpedantic -Werror)
#endif()

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
# set(JSON_NOEXCEPTION ON)
set(JSON_BuildTests OFF CACHE INTERNAL "")

# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
set(JSON_Install OFF CACHE INTERNAL "")

# Don't use include(nlohmann_json/CMakeLists.txt) since that carries with it
# unintended consequences that will break the build.  It's generally
# discouraged (although not necessarily well documented as such) to use
# include(...) for pulling in other CMake projects anyways.
add_subdirectory("external/nlohmann_json")

set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE) #example if don't need the graph tool
set(FASTNOISE2_TESTS     OFF CACHE BOOL "idk" FORCE)
set(FASTNOISE2_UTILITY   OFF CACHE BOOL "idk" FORCE)

add_subdirectory(external/fastnoise2)
add_subdirectory(external/flecs)
add_subdirectory(external/dylib)

target_include_directories(VoxelGame PRIVATE "src")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/fastnoise2/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/vma/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/vkbootstrap")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/imgui")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/imgui/backends")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/nlohmann_json/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/magic_enum/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/flecs/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/ctre")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/concurrentqueue")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/dylib/include")
target_include_directories(VoxelGame SYSTEM PRIVATE "external/boost")

#if(USE_FEATURE_X)
    #add_definitions(-DFEATURE_X_ENABLED)
    #target_sources(myTarget PRIVATE feature_x.cpp)
#else()

target_link_libraries(VoxelGame PRIVATE glfw Vulkan::Vulkan fmt::fmt nlohmann_json::nlohmann_json FastNoise flecs::flecs_static vmlib dylib)

target_compile_options(flecs PRIVATE -w)
#target_compile_options(simdjson PRIVATE -w)

target_compile_options(VoxelGame PRIVATE -Wnrvo)

#endif()

#set_target_properties(flecs PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:flecs,INTERFACE_INCLUDE_DIRECTORIES>)

if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()
