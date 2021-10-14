#pragma once
#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <imgui.h>
#include <entt/entt.hpp>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <unordered_map>
#include <memory>
#include <vector>
#include <list>
#include <array>
#include <set>
#include <queue>
#include <typeinfo>
#include <stdexcept>
#include <functional>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error please compile using c++ 17
#endif
#include <initializer_list>
#include <utility>
#include <tuple>
#include <optional>
#include <limits>
// everything should include ref
#include "ref.h"