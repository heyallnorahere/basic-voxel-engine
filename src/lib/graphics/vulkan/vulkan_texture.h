#pragma once
#include "graphics/texture.h"
#include "vulkan_object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_texture : public texture {
            public:
                vulkan_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, ref<vulkan_object_factory> factory);
                virtual ~vulkan_texture() override;
                virtual void bind(uint32_t slot) override;
                virtual glm::ivec2 get_size() override;
                virtual int32_t get_channels() override;
                virtual void* get_id() override;
            private:
                ref<vulkan_object_factory> m_factory;
                VkDevice m_device;
                int32_t m_width, m_height, m_channels;
            };
        }
    }
}