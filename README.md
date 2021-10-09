# basic-voxel-engine [![Build status](https://img.shields.io/github/workflow/status/yodasoda1219/basic-voxel-engine/build)](https://github.com/yodasoda1219/basic-voxel-engine/actions/workflows/build.yml) [![Total alerts](https://img.shields.io/lgtm/alerts/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/alerts/) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/yodasoda1219/basic-voxel-engine)](https://lgtm.com/projects/g/yodasoda1219/basic-voxel-engine/context:cpp)

A Minecraft-like voxel engine, written in C++. To use the library on its own without the application, include this repository as a submodule in your project, add it to your build via CMake's `add_subdirectory()`, and link against the `basic-voxel-engine` target.

## Project Structure

- `src`: core C++ BVE code and assets
- `BasicVoxelEngine`: core C# scripting library
- `BasicVoxelEngine.Content`: C# BVE content (blocks, etc.)
- `vendor`: submodule dependencies

## Dependencies

Submodule dependencies can be synced by running `git submodule update --init --recursive`. The following projects, however, need to be installed manually:

- [mono](https://www.mono-project.com/download/stable)\*
- [Vulkan](#installing-vulkan)
- [Python 3](https://www.python.org/downloads/)

\* On Arch Linux, the `mono` package is outdated. Install `mono-git` from the AUR and `mono-msbuild` from the Community repository instead.

## Installing Vulkan

On Windows, MacOS X, and Ubuntu, run [this Python script](scripts/setup_vulkan.py) to install Vulkan. Otherwise, install Vulkan from [here](https://vulkan.lunarg.com/sdk/home).