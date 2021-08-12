#include "bve_pch.h"
#include "window.h"
#include "world.h"
#include "block.h"
#include "shader.h"
#include "mesh_factory.h"
#include "renderer.h"
#include "components.h"
#include "asset_manager.h"
#include "input_manager.h"
namespace bve {
    using callback = std::function<void()>;
    struct render_data {
        callback clear;
        callback swap_buffers;
        std::shared_ptr<world> world_;
        std::shared_ptr<renderer> renderer_;
        std::shared_ptr<shader> shader_;
        float aspect_ratio;
        std::shared_ptr<texture_atlas> atlas;
        std::vector<std::vector<mesh_factory::processed_voxel>> clusters;
        std::shared_ptr<input_manager> input_manager_;
    };
    static entity player;
    static void create_player(std::shared_ptr<world> world_) {
        player = world_->create();
        player.get_component<components::transform_component>().translation = glm::vec3(5.f);
        player.add_component<components::camera_component>().direction = glm::vec3(-1.f);
    }
    static void update(std::shared_ptr<world> world_, std::shared_ptr<input_manager> input_manager_) {
        input_manager_->update();
    }
    static void render(const render_data& data) {
        data.clear();
        auto cmdlist = data.renderer_->create_command_list();
        mesh_factory factory(data.world_);
        for (auto& cluster : data.clusters) {
            auto mesh_ = factory.create_mesh(cluster);
            data.renderer_->add_mesh(cmdlist, mesh_);
        }
        data.renderer_->close_command_list(cmdlist, factory.get_vertex_attributes());
        data.renderer_->set_camera_data(player, data.aspect_ratio);
        data.renderer_->render(cmdlist, data.shader_, data.atlas);
        data.renderer_->destroy_command_list(cmdlist);
#ifndef NDEBUG
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
            ImGui::Checkbox("Mouse enabled", &data.input_manager_->mouse_enabled());
            ImGui::Text("FPS: %f", io.Framerate);
            ImGui::Image((ImTextureID)(size_t)data.atlas->get_texture()->get_id(), { 100, 100 });
            ImGui::End();
        }
#endif
        data.swap_buffers();
    }
    static void main_loop(std::shared_ptr<window> window_, std::shared_ptr<world> world_) {
        callback clear = [window_]() { window_->clear(); };
        callback swap_buffers = [window_]() { window_->swap_buffers(); };
        asset_manager& asset_manager_ = asset_manager::get();
        asset_manager_.reload({ std::filesystem::current_path() / "assets" });
        auto atlas = asset_manager::get().create_texture_atlas();
        auto shader_ = shader::create({ { asset_manager_.get_asset_path("shaders:vertex.glsl").string(), GL_VERTEX_SHADER }, { asset_manager_.get_asset_path("shaders:fragment.glsl").string(), GL_FRAGMENT_SHADER } });
        auto renderer_ = std::make_shared<renderer>();
        auto input_manager_ = std::make_shared<input_manager>(window_);
        create_player(world_);
        auto clusters = mesh_factory(world_).get_clusters();
        world_->on_block_changed([&](glm::ivec3, std::shared_ptr<world>) { clusters = mesh_factory(world_).get_clusters(); });
        while (!window_->should_close()) {
            window_->new_frame();
            update(world_, input_manager_);
            glm::vec2 size = glm::vec2(window_->get_framebuffer_size());
            render({ clear, swap_buffers, world_, renderer_, shader_, size.x / size.y, atlas, clusters, input_manager_ });
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