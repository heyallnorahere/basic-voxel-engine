# basic-voxel-engine [![Build status](https://img.shields.io/github/workflow/status/yodasoda1219/basic-voxel-engine/build)](https://github.com/yodasoda1219/basic-voxel-engine/actions/workflows/build.yml) [![Total alerts](https://img.shields.io/lgtm/alerts/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/alerts/) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/context:cpp)

A Minecraft-like voxel engine, written in C++. To use the library on its own without the application, include this repository as a submodule in your project and add it to your build via CMake's `add_subdirectory()` and link against the `basic-voxel-engine` target.

## Dependencies

- [Assimp](https://github.com/assimp/assimp/tree/v5.0.1)
- [CMake](https://cmake.org)
- [entt](https://github.com/skypjack/entt/tree/v3.8.1)
- [glad](https://github.com/Dav1dde/glad)
- [GLFW](https://github.com/glfw/glfw/tree/3.3.4)
- [GLM](https://github.com/g-truc/glm/tree/0.9.9.8)
- [glslang](https://github.com/KhronosGroup/glslang)
- [Dear ImGui](https://github.com/ocornut/imgui/tree/docking)
- [spdlog](https://github.com/gabime/spdlog/tree/v1.9.1)
- [stb](https://github.com/nothings/stb)
- [mono](https://www.mono-project.com/download/stable)
- [Vulkan](#installing-vulkan)

## Installing Vulkan

On Windows, MacOS X, and Ubuntu, run [this Python script](scripts/setup_vulkan.py) to install Vulkan. Otherwise, install Vulkan from [here](https://vulkan.lunarg.com/sdk/home).