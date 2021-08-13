#include "bve_pch.h"
#include "player.h"
#include "components.h"
#include "application.h"
using namespace bve;
static float camera_sensitivity = 0.1f;
void player::on_attach() {
    this->get_component<components::transform_component>().translation = glm::vec3(5.f);
    this->add_component<components::camera_component>().direction = glm::vec3(-1.f);
}
void player::update() {
    this->update_camera_direction();
    this->take_input();
#ifndef NDEBUG
    {
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
        ImGui::DragFloat3("Position", &transform.translation.x);
        ImGui::DragFloat3("Camera direction", &camera.direction.x, 1.f, 0.f, 0.f, "%.3f", lock_camera ? ImGuiSliderFlags_NoInput : ImGuiSliderFlags_None);
        ImGui::SliderFloat("Camera sensitivity", &camera_sensitivity, 0.f, 0.5f);
        ImGui::Checkbox("Lock camera", &lock_camera);
        ImGui::Text("FPS: %f", io.Framerate);
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
