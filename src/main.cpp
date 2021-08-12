#include "bve_pch.h"
#include "window.h"
#include "world.h"
#include "block.h"
#include "shader.h"
#include "mesh_factory.h"
#include "renderer.h"
#include "components.h"
#include "asset_manager.h"
namespace bve {
    using callback = std::function<void()>;
    static entity player;
    static void create_player(std::shared_ptr<world> world_) {
        player = world_->create();
        player.get_component<components::transform_component>().translation = glm::vec3(5.f);
        player.add_component<components::camera_component>().direction = glm::vec3(-1.f);
    }
    static void update(std::shared_ptr<world> world_) {
        // todo: update
    }
    static void render(callback clear, callback swap_buffers, std::shared_ptr<world> world_, std::shared_ptr<renderer> renderer_, std::shared_ptr<shader> shader_, float aspect_ratio, std::shared_ptr<texture_atlas> atlas) {
        clear();
        auto cmdlist = renderer_->create_command_list();
        mesh_factory factory(world_);
        auto clusters = factory.get_clusters();
        for (auto& cluster : clusters) {
            GLuint vertex_buffer, index_buffer;
            size_t index_count;
            factory.create_mesh(cluster, vertex_buffer, index_buffer, index_count);
            renderer_->add_mesh(cmdlist, vertex_buffer, index_buffer, index_count);
        }
        renderer_->close_command_list(cmdlist, factory.get_vertex_attributes());
        renderer_->set_camera_data(player, aspect_ratio);
        renderer_->render(cmdlist, shader_, atlas);
        renderer_->destroy_command_list(cmdlist);
        {
            auto& transform = player.get_component<components::transform_component>();
            auto& camera = player.get_component<components::camera_component>();
            static bool lock_camera = false;
            static std::shared_ptr<glm::vec3> original_direction;
            if (lock_camera) {
                if (!original_direction) {
                    original_direction = std::make_shared<glm::vec3>(camera.direction);
                }
                camera.direction = glm::normalize(-transform.translation);
            } else if (original_direction) {
                camera.direction = *original_direction;
                original_direction.reset();
            }
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin("Player control");
            ImGui::DragFloat3("Position", &transform.translation.x);
            ImGui::DragFloat3("Camera direction", &camera.direction.x, 1.f, 0.f, 0.f, "%.3f", lock_camera ? ImGuiSliderFlags_NoInput : ImGuiSliderFlags_None);
            ImGui::Checkbox("Lock camera", &lock_camera);
            ImGui::Text("FPS: %f", io.Framerate);
            ImGui::Image((ImTextureID)(size_t)atlas->get_texture()->get_id(), { 100, 100 });
            ImGui::End();
        }
        swap_buffers();
    }
    static void main_loop(std::shared_ptr<window> window_, std::shared_ptr<world> world_) {
        callback clear = [window_]() { window_->clear(); };
        callback swap_buffers = [window_]() { window_->swap_buffers(); };
        asset_manager& asset_manager_ = asset_manager::get();
        asset_manager_.reload({ std::filesystem::current_path() / "assets" });
        auto atlas = asset_manager::get().create_texture_atlas();
        auto shader_ = shader::create({ { asset_manager_.get_asset_path("shaders:vertex.glsl").string(), GL_VERTEX_SHADER }, { asset_manager_.get_asset_path("shaders:fragment.glsl").string(), GL_FRAGMENT_SHADER } });
        auto renderer_ = std::make_shared<renderer>();
        create_player(world_);
        while (!window_->should_close()) {
            window_->new_frame();
            update(world_);
            glm::vec2 size = glm::vec2(window_->get_framebuffer_size());
            render(clear, swap_buffers, world_, renderer_, shader_, size.x / size.y, atlas);
            bve::window::poll_events();
        }
    }
}
int main(int argc, const char** argv) {
    try {
        using namespace bve;
        auto window_ = std::make_shared<window>(800, 600);
        block::register_all();
        auto world_ = std::make_shared<world>(glm::ivec3(16, 256, 16));
        main_loop(window_, world_);
        return EXIT_SUCCESS;
    } catch (const std::runtime_error& exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
}