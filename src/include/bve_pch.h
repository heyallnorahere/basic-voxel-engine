#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <entt/entt.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <stddef.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <list>
#include <typeinfo>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include <initializer_list>
#include <utility>
#include <tuple>
// saving this for later
/*
#ifdef BVE_PLATFORM_WINDOWS
#include <Windows.h>
#define bve_dlopen LoadLibraryA
#define bve_dlclose FreeLibrary
#define bve_dlsym GetProcAddress
using bve_dll = HMODULE;
#endif
#ifdef BVE_PLATFORM_UNIX
#include <dlfcn.h>
#define bve_dlopen(path) dlopen(path, RTLD_LAZY)
#define bve_dlclose dlclose
#define bve_dlsym dlsym
using bve_dll = void*;
#endif
*/