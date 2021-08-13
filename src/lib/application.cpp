#include "bve_pch.h"
#include "application.h"
#include "block.h"
#include "asset_manager.h"
namespace bve {
    using callback = std::function<void()>;
    application& application::get() {
        static application instance;
        return instance;
    }
    void application::run() {
        callback clear = [this]() { this->m_window->clear(); };
        callback swap_buffers = [this]() { this->m_window->swap_buffers(); };
        this->m_clusters = mesh_factory(this->m_world).get_clusters();
        this->m_world->on_block_changed([this](glm::ivec3, ref<world>) { this->m_clusters = mesh_factory(this->m_world).get_clusters(); });
        this->m_running = true;
        while (!this->m_window->should_close() && this->m_running) {
            this->m_window->new_frame();
            update();
            render();
            bve::window::poll_events();
        }
    }
    void application::quit() {
        this->m_running = false;
    }
    double application::get_delta_time() {
        return this->m_delta_time;
    }
    ref<shader> application::get_shader(const std::string& name) {
        if (this->m_shaders.find(name) == this->m_shaders.end()) {
            throw std::runtime_error("[application] the specified shader does not exist");
        }
        return this->m_shaders[name];
    }
    application::application() {
        block::register_all();
        asset_manager& asset_manager_ = asset_manager::get();
        asset_manager_.reload({ std::filesystem::current_path() / "assets" });
        this->m_world = ref<world>::create(glm::ivec3(16, 16, 256));
        this->m_window = ref<window>::create(800, 600);
        this->m_atlas = asset_manager_.create_texture_atlas();
        this->m_shaders["block"] = shader::create({ { asset_manager_.get_asset_path("shaders:vertex.glsl").string(), GL_VERTEX_SHADER }, { asset_manager_.get_asset_path("shaders:fragment.glsl").string(), GL_FRAGMENT_SHADER } });
        this->m_renderer = ref<renderer>::create();
        this->m_input_manager = ref<input_manager>::create(this->m_window);
        this->m_running = false;
        this->m_delta_time = 0.0;
        this->m_last_frame = glfwGetTime();
    }
    void application::update() {
        double current_frame = glfwGetTime();
        this->m_delta_time = current_frame - this->m_last_frame;
        this->m_last_frame = current_frame;
        this->m_input_manager->update();
    }
    void application::render() {
        this->m_window->clear();
        auto cmdlist = this->m_renderer->create_command_list();
        mesh_factory factory(this->m_world);
        for (auto& cluster : this->m_clusters) {
            auto mesh_ = factory.create_mesh(cluster);
            this->m_renderer->add_mesh(cmdlist, mesh_);
        }
        this->m_renderer->close_command_list(cmdlist, factory.get_vertex_attributes());
        //data.renderer_->set_camera_data(player, data.aspect_ratio);
        this->m_renderer->render(cmdlist, this->m_shaders["block"], this->m_atlas);
        this->m_renderer->destroy_command_list(cmdlist);
/*#ifndef NDEBUG
        {
            auto& transform = player.get_component<components::transform_component>();
            auto& camera = player.get_component<components::camera_component>();
            static bool lock_camera = false;
            static ref<glm::vec3> original_direction;
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
            ImGui::Checkbox("Mouse enabled", &data.input_manager_->mouse_enabled());
            ImGui::Text("FPS: %f", io.Framerate);
            ImGui::Image((ImTextureID)(size_t)data.atlas->get_texture()->get_id(), { 100, 100 });
            ImGui::End();
        }
#endif*/
        this->m_window->swap_buffers();
    }
}