#include "bve_pch.h"
#include "vulkan_texture.h"
#include "util.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_texture::vulkan_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, ref<vulkan_object_factory> factory) {
                this->m_width = width;
                this->m_height = height;
                this->m_channels = channels;
                this->m_factory = factory;
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                this->m_device = context->get_device();
                this->create(data, context);
            }
            vulkan_texture::~vulkan_texture() {
                // todo: clean up
            }
            void vulkan_texture::bind(uint32_t slot) {
                // todo: bind
            }
            glm::ivec2 vulkan_texture::get_size() {
                return glm::ivec2(this->m_width, this->m_height);
            }
            int32_t vulkan_texture::get_channels() {
                return this->m_channels;
            }
            ImTextureID vulkan_texture::get_texture_id() {
                return nullptr; // todo: figure out some ImGui thing
            }
            void vulkan_texture::create(const std::vector<uint8_t>& data, ref<context> context) {
                auto vk_context = context.as<vulkan_context>();
                spdlog::warn("vulkan textures are not implemented yet");
            }
        }
    }
}