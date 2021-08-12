#include "bve_pch.h"
#include "texture_atlas.h"
#include "block.h"
namespace bve {
        void texture_atlas::set_uniform(std::shared_ptr<shader> shader_, const std::string& uniform_name, GLint texture_slot) {
        this->m_texture->bind(texture_slot);
        auto get_uniform_name = [uniform_name](const std::string& field_name) {
            return uniform_name + "." + field_name;
        };
        shader_->bind();
        shader_->set_uniform(get_uniform_name("texture"), texture_slot);
        shader_->set_uniform(get_uniform_name("texture_size"), this->m_texture_size);
        shader_->set_uniform(get_uniform_name("atlas_size"), this->m_atlas_size);
        auto& block_register = registry::get().get_register<block>();
        for (const std::pair<namespaced_name, std::pair<glm::ivec2, glm::ivec2>>& pair : this->m_texture_dimensions) {
            std::optional<size_t> index = block_register.get_index(pair.first);
            if (!index) {
                throw std::runtime_error("[texture atlas] the specified block does not exist");
            }
            std::string element_name = get_uniform_name("texture_dimensions_array[" + std::to_string(*index) + "]");
            shader_->set_uniform(element_name + ".atlas_position", pair.second.first);
            shader_->set_uniform(element_name + ".texture_dimensions", pair.second.second);
        }
    }
    glm::ivec2 texture_atlas::get_texture_size() {
        return this->m_texture_size;
    }
    glm::ivec2 texture_atlas::get_atlas_size() {
        return this->m_atlas_size;
    }
    std::shared_ptr<texture> texture_atlas::get_texture() {
        return this->m_texture;
    }
    texture_atlas::texture_atlas(const std::vector<std::pair<namespaced_name, texture_data>>& textures) {
        std::vector<std::pair<namespaced_name, texture_data>> textures_copy(textures.size());
        std::copy(textures.begin(), textures.end(), textures_copy.begin());
        // i know this code is dubious but its the best i could think of
        constexpr int32_t max_block_width = 10;
        constexpr int32_t channels = 4;
        int32_t texture_count = (int32_t)textures_copy.size();
        if (texture_count <= max_block_width) {
            this->m_atlas_size = glm::ivec2(texture_count, 1);
        } else {
            int32_t remainder_ = texture_count % max_block_width;
            this->m_atlas_size.x = max_block_width;
            this->m_atlas_size.y = ((texture_count - remainder_) / max_block_width) + (remainder_ > 0 ? 1 : 0);
        }
        glm::ivec2 largest_texture_size = glm::ivec2(0);
        for (auto& pair : textures_copy) {
            if (pair.second.channels != channels) {
                if (pair.second.channels > channels) {
                    throw std::runtime_error("[texture atlas] the image cannot have more than " + std::to_string(channels) + " channels");
                }
                // this is very hacky but i had to make it work
                std::vector<uint8_t> new_data(pair.second.data.size());
                std::copy(pair.second.data.begin(), pair.second.data.end(), new_data.begin());
                for (size_t i = pair.second.channels; i < new_data.size() + 1; i += channels) {
                    auto it = new_data.begin();
                    std::advance(it, i);
                    std::vector<uint8_t> filler_data((size_t)channels - pair.second.channels);
                    memset(filler_data.data(), 255, filler_data.size() * sizeof(uint8_t));
                    new_data.insert(it, filler_data.begin(), filler_data.end());
                }
                pair.second.data = new_data;
                pair.second.channels = channels;
            }
            glm::ivec2 texture_size = glm::ivec2(pair.second.width, pair.second.height);
            if (texture_size.x > largest_texture_size.x) {
                largest_texture_size.x = texture_size.x;
            }
            if (texture_size.y > largest_texture_size.y) {
                largest_texture_size.y = texture_size.y;
            }
        }
        this->m_texture_size = largest_texture_size * this->m_atlas_size;
        size_t buffer_size = (size_t)this->m_texture_size.x * this->m_texture_size.y * channels;
        std::vector<uint8_t> atlas_texture_data(buffer_size);
        for (size_t i = 0; i < textures_copy.size(); i++) {
            glm::ivec2 atlas_position;
            if (this->m_atlas_size.y > 1) {
                int32_t index = (int32_t)i;
                atlas_position.x = index % this->m_atlas_size.x;
                atlas_position.y = (index - atlas_position.x) / this->m_atlas_size.x;
            } else {
                atlas_position = glm::ivec2((int32_t)i, 0);
            }
            const auto& data = textures_copy[i].second;
            glm::ivec2 texture_size = glm::ivec2(data.width, data.height);
            this->m_texture_dimensions[textures_copy[i].first] = { atlas_position, texture_size };
            for (int32_t y = 0; y < texture_size.y; y++) {
                glm::ivec2 atlas_buffer_position = (atlas_position * largest_texture_size) + glm::ivec2(0, y);
                size_t atlas_buffer_index = (((size_t)atlas_buffer_position.y * this->m_texture_size.x) + atlas_buffer_position.x) * channels;
                size_t texture_buffer_index = ((size_t)y * texture_size.x) * channels;
                for (int32_t x = 0; x < texture_size.x * channels; x++) {
                    size_t index_offset = (size_t)x;
                    atlas_texture_data[atlas_buffer_index + index_offset] = data.data[texture_buffer_index + index_offset];
                }
            }
        }
        texture_settings settings;
        // todo: texture settings
        this->m_texture = std::make_shared<texture>(atlas_texture_data, this->m_texture_size.x, this->m_texture_size.y, channels, settings);
    }
}