cmake_minimum_required(VERSION 4.5.1)

project(ACE VERSION 0.1.0 LANGUAGES C CXX)

add_executable(ACE)

add_subdirectory(thirdparty)
add_subdirectory(src)

target_link_libraries(ACE glad Imgui Imgui_backend)

target_link_libraries(ACE SDL2 SDL2_mixer GL m freeimageplus assimp tbb SPIRV glslang-default-resource-limits glslang)
