cmake_minimum_required(VERSION "3.19.2")

option(PRODUCTION_BUILD "Build in production mode" OFF)

project(VoxelGame)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fmt REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/vkbootstrap/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui*.cpp")
#if(NOT ENABLE_FEATURE_B)
#    file(GLOB_RECURSE FEATURE_B_FILES CONFIGURE_DEPENDS src/featureB/*.cpp src/featureB/*.h)
#    list(REMOVE_ITEM SRC_FILES ${FEATURE_B_FILES})
#endif()

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

target_include_directories(VoxelGame PRIVATE "src")
target_include_directories(VoxelGame PRIVATE "external/fastnoiselite")
target_include_directories(VoxelGame PRIVATE "external/vma/include")
target_include_directories(VoxelGame PRIVATE "external/vkbootstrap")
target_include_directories(VoxelGame PRIVATE "external/imgui")
target_include_directories(VoxelGame PRIVATE "external/imgui/backends")
target_include_directories(VoxelGame PRIVATE "external/nlohmann_json/include")
target_include_directories(VoxelGame PRIVATE "external/ctre")

#if(USE_FEATURE_X)
    #add_definitions(-DFEATURE_X_ENABLED)
    #target_sources(myTarget PRIVATE feature_x.cpp)
#else()
target_link_libraries(VoxelGame PRIVATE glfw Vulkan::Vulkan fmt::fmt nlohmann_json::nlohmann_json)
#endif()

if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()
