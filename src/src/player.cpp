#include "bve_pch.h"
#include "player.h"
#include "components.h"
#include "application.h"
#include "block.h"
using namespace bve;
static float camera_sensitivity = 0.1f;
static void swap(std::vector<namespaced_name>& names, size_t index_1, size_t index_2) {
    auto v1 = names[index_1];
    auto v2 = names[index_2];
    names[index_1] = v2;
    names[index_2] = v1;
}
static size_t partition(std::vector<namespaced_name>& names, size_t low, size_t high, object_register<block>& block_register) {
    namespaced_name pivot = names[high];
    size_t i = low - 1;
    for (size_t j = low; j < high; j++) {
        namespaced_name current_name = names[j];
        if (*block_register.get_index(current_name) < *block_register.get_index(pivot)) {
            i++;
            swap(names, i, j);
        }
    }
    size_t partitioning_index = i + 1;
    swap(names, partitioning_index, high);
    return partitioning_index;
}
static void quick_sort(std::vector<namespaced_name>& names, int32_t low, int32_t high, object_register<block>& block_register) {
    if (low < high) {
        int32_t partitioning_index = (int32_t)partition(names, (size_t)low, (size_t)high, block_register);
        quick_sort(names, low, partitioning_index - 1, block_register);
        quick_sort(names, partitioning_index + 1, high, block_register);
    }
}
void player::on_attach() {
    this->get_component<components::transform_component>().translation = glm::vec3(5.f);
    this->add_component<components::camera_component>().direction = glm::vec3(-1.f);
}
void player::update() {
    this->update_camera_direction();
    this->take_input();
#ifndef NDEBUG
    {
        auto& app = application::get();
        auto& block_register = registry::get().get_register<block>();
        auto& transform = this->get_component<components::transform_component>();
        auto& camera = this->get_component<components::camera_component>();
        static bool lock_camera = false;
        static std::shared_ptr<glm::vec3> original_direction;
        if (lock_camera) {
            if (!original_direction) {
                original_direction = std::make_shared<glm::vec3>(camera.direction);
            }
            camera.direction = glm::normalize(-transform.translation);
        }
        else if (original_direction) {
            camera.direction = *original_direction;
            original_direction.reset();
        }
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Begin("Player control");
        ImVec2 content_region_available = ImGui::GetContentRegionAvail();
        ImGui::DragFloat3("Position", &transform.translation.x);
        ImGui::DragFloat3("Camera direction", &camera.direction.x, 1.f, 0.f, 0.f, "%.3f", lock_camera ? ImGuiSliderFlags_NoInput : ImGuiSliderFlags_None);
        ImGui::SliderFloat("Camera sensitivity", &camera_sensitivity, 0.f, 0.5f);
        ImGui::Checkbox("Lock camera", &lock_camera);
        ImGui::Text("FPS: %f", io.Framerate);
        auto atlas = app.get_texture_atlas();
        if (ImGui::CollapsingHeader("Texture atlas")) {
            void* texture_id = atlas->get_texture()->get_id();
            glm::ivec2 size = atlas->get_texture_size();
            ImVec2 bottom_left, top_right;
            if (app.get_object_factory()->get_graphics_api() == graphics::graphics_api::OPENGL) {
                bottom_left = ImVec2(0.f, 1.f);
                top_right = ImVec2(1.f, 0.f);
            } else {
                bottom_left = ImVec2(0.f, 0.f);
                top_right = ImVec2(1.f, 1.f);
            }
            ImGui::Image(texture_id, ImVec2(content_region_available.x, 100.f), bottom_left, top_right);
        }
        if (ImGui::CollapsingHeader("Texture atlas uniform data")) {
            static int32_t current_combo_item = 0;
            auto shader = app.get_shader("block");
            shader->bind();
            glm::ivec2 texture_size = shader->get_ivec2("atlas.texture_size");
            glm::ivec2 atlas_size = shader->get_ivec2("atlas.atlas_size");
            std::vector<namespaced_name> names = atlas->get_included_block_names();
            quick_sort(names, 0, names.size() - 1, block_register);
            std::vector<char> combo_names_characters;
            for (const auto& name : names) {
                std::string full_name = name.get_full_name();
                for (char c : full_name) {
                    combo_names_characters.push_back(c);
                }
                combo_names_characters.push_back('\0');
            }
            combo_names_characters.push_back('\0');
            ImGui::Text("Texture size: (%d, %d)", texture_size.x, texture_size.y);
            ImGui::Text("Atlas size: (%d, %d)", atlas_size.x, atlas_size.y);
            ImGui::Combo("Current block", &current_combo_item, combo_names_characters.data());
            size_t block_id = *block_register.get_index(names[(size_t)current_combo_item]);
            std::string uniform_stub = "atlas.texture_dimensions_array[" + std::to_string(block_id) + "].";
            glm::ivec2 atlas_position = shader->get_ivec2(uniform_stub + "atlas_position");
            glm::ivec2 texture_dimensions = shader->get_ivec2(uniform_stub + "texture_dimensions");
            ImGui::Text("Block atlas position: (%d, %d)", atlas_position.x, atlas_position.y);
            ImGui::Text("Block texture dimensions: (%d, %d)", texture_dimensions.x, texture_dimensions.y);
        }
        ImGui::End();
    }
#endif
}
void player::update_camera_direction() {
    auto& app = application::get();
    ref<input_manager> input_manager_ = app.get_input_manager();
    glm::vec2 offset = input_manager_->get_mouse() * camera_sensitivity;
    auto& camera = this->get_component<components::camera_component>();
    glm::vec2 radians;
    radians.x = asin(camera.direction.y);
    float factor = cos(radians.x);
    radians.y = atan2(camera.direction.z / factor, camera.direction.x / factor);
    glm::vec2 angle = glm::degrees(radians);
    angle += glm::vec2(offset.y, offset.x);
    if (angle.x > 89.f) {
        angle.x = 89.f;
    }
    if (angle.x < -89.f) {
        angle.x = -89.f;
    }
    radians = glm::radians(angle);
    glm::vec3 direction;
    direction.x = cos(radians.x) * cos(radians.y);
    direction.y = sin(radians.x);
    direction.z = cos(radians.x) * sin(radians.y);
    camera.direction = glm::normalize(direction);
}
void player::take_input() {
    auto& app = application::get();
    ref<input_manager> input_manager_ = app.get_input_manager();
    auto& camera = this->get_component<components::camera_component>();
    auto& transform = this->get_component<components::transform_component>();
    float player_speed = 2.5f * (float)app.get_delta_time();
    glm::vec3 forward = camera.direction * player_speed;
    glm::vec3 right = glm::normalize(glm::cross(camera.direction, camera.up)) * player_speed;
    if (input_manager_->get_key(GLFW_KEY_W).held) {
        transform.translation += forward;
    }
    if (input_manager_->get_key(GLFW_KEY_S).held) {
        transform.translation -= forward;
    }
    if (input_manager_->get_key(GLFW_KEY_A).held) {
        transform.translation -= right;
    }
    if (input_manager_->get_key(GLFW_KEY_D).held) {
        transform.translation += right;
    }
    if (input_manager_->get_key(GLFW_KEY_E).down) {
        bool& mouse_enabled = input_manager_->mouse_enabled();
        mouse_enabled = !mouse_enabled;
    }
    if (input_manager_->get_key(GLFW_KEY_Q).down) {
        app.quit();
    }
}
