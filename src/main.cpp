#include "bve_pch.h"
#include "components.h"
#include "application.h"
namespace bve {
    static entity player;
    static float camera_sensitivity = 0.1f;
    static void create_player(std::shared_ptr<world> world_) {
        player = world_->create();
        player.get_component<components::transform_component>().translation = glm::vec3(5.f);
        player.add_component<components::camera_component>().direction = glm::vec3(-1.f);
    }
    static void update_player_camera_direction(glm::vec2 mouse) {
        glm::vec2 offset = mouse * camera_sensitivity;
        auto& camera = player.get_component<components::camera_component>();
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
    static void move_player(std::shared_ptr<input_manager> input_manager_, float delta_time) {
        auto& camera = player.get_component<components::camera_component>();
        auto& transform = player.get_component<components::transform_component>();
        float player_speed = 2.5f * delta_time;
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
    }
}
int main(int argc, const char** argv) {
    try {
        using namespace bve;
        auto& app = application::get();
        // todo: player stuff
        app.run();
        return EXIT_SUCCESS;
    } catch (const std::runtime_error& exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
}