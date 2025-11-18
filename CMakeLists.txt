cmake_minimum_required(VERSION "3.19.2")

option(PRODUCTION_BUILD "Build in production mode" OFF)

project(VoxelGame)
set(CMAKE_CXX_STANDARD 23)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fmt REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/vkbootstrap/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui*.cpp")
#if(NOT ENABLE_FEATURE_B)
#    file(GLOB_RECURSE FEATURE_B_FILES CONFIGURE_DEPENDS src/featureB/*.cpp src/featureB/*.h)
#    list(REMOVE_ITEM SRC_FILES ${FEATURE_B_FILES})
#endif()

add_executable(VoxelGame "${MY_SOURCES}")

target_include_directories(VoxelGame PRIVATE "src")
target_include_directories(VoxelGame PRIVATE "external/fastnoiselite")
target_include_directories(VoxelGame PRIVATE "external/vma/include")
target_include_directories(VoxelGame PRIVATE "external/vkbootstrap")
target_include_directories(VoxelGame PRIVATE "external/imgui")
target_include_directories(VoxelGame PRIVATE "external/imgui/backends")

#if(USE_FEATURE_X)
    #add_definitions(-DFEATURE_X_ENABLED)
    #target_sources(myTarget PRIVATE feature_x.cpp)
#else()
target_link_libraries(VoxelGame PRIVATE glfw Vulkan::Vulkan fmt::fmt)
#endif()

if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()
