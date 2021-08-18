# basic-voxel-engine [![Build status](https://img.shields.io/github/workflow/status/yodasoda1219/basic-voxel-engine/build)](https://github.com/yodasoda1219/basic-voxel-engine/actions/workflows/build.yml) [![Total alerts](https://img.shields.io/lgtm/alerts/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/alerts/) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/context:cpp)

A Minecraft-like voxel engine, written in C++. To use the library on its own without the application, include this repository as a submodule in your project and add it to your build via CMake's `add_subdirectory()` and link against the `basic-voxel-engine` target.

## Dependencies

- [Assimp](vendor/assimp)
- [CMake](https://cmake.org)
- [entt](vendor/entt)
- [glad](vendor/glad)
- [GLFW](vendor/glfw)
- [GLM](vendor/glm)
- [Dear ImGui](vendor/imgui)
- [spdlog](vendor/spdlog)
- [stb](vendor/stb)
- [mono](https://www.mono-project.com/download/stable)