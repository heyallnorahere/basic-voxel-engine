#include "bve_pch.h"
#include "application.h"
#include "block.h"
#include "asset_manager.h"
#include "components.h"
namespace bve {
    application& application::get() {
        static application instance;
        return instance;
    }
    void application::run() {
        auto& block_register = registry::get().get_register<block>();
        for (const auto& name : block_register.get_names()) {
            ref<block> block_ = block_register[name];
            block_->load(this->m_object_factory, name);
        }
#ifndef NDEBUG
        {
            auto app_class = this->m_code_host->find_class("BasicVoxelEngine.Application");
            auto testmethod = app_class->get_method("*:TestMethod");
            app_class->invoke(testmethod);
        }
#endif
        this->m_clusters = mesh_factory(this->m_world).get_clusters(this->m_lights);
        this->m_world->on_block_changed([this](glm::ivec3, ref<world>) { this->m_clusters = mesh_factory(this->m_world).get_clusters(this->m_lights); });
        this->m_running = true;
        while (!this->m_window->should_close() && this->m_running) {
            this->m_window->new_frame();
            this->update();
            this->render();
            bve::window::poll_events();
        }
    }
    void application::quit() {
        this->m_running = false;
    }
    double application::get_delta_time() {
        return this->m_delta_time;
    }
    ref<graphics::shader> application::get_shader(const std::string& name) {
        if (this->m_shaders.find(name) == this->m_shaders.end()) {
            throw std::runtime_error("[application] the specified shader does not exist");
        }
        return this->m_shaders[name];
    }
    application::application() {
        spdlog::info("[application] starting BVE, working directory: {0}", std::filesystem::current_path().string());
        this->m_code_host = ref<code_host>::create();
        this->load_assemblies();
        {
            auto app_class = this->m_code_host->find_class("BasicVoxelEngine.Application");
            auto load_content = app_class->get_method("*:LoadContent");
            app_class->invoke(load_content);
        }
        this->m_object_factory = graphics::object_factory::create(graphics::graphics_api::OPENGL); // todo: switch with cmake options
        asset_manager& asset_manager_ = asset_manager::get();
        asset_manager_.reload({ std::filesystem::current_path() / "assets" });
        this->m_world = ref<world>::create();
        this->m_window = ref<window>::create(1600, 900, this->m_object_factory->create_context());
        this->m_atlas = asset_manager_.create_texture_atlas(this->m_object_factory);
        this->m_shaders["block"] = this->m_object_factory->create_shader({ asset_manager_.get_asset_path("shaders:static.glsl") });
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
        this->m_world->update();
    }
    void application::render() {
        this->m_window->get_context()->clear();
        auto cmdlist = this->m_renderer->create_command_list();
        mesh_factory factory(this->m_world);
        for (auto& cluster : this->m_clusters) {
            auto mesh_ = factory.create_mesh(cluster);
            this->m_renderer->add_mesh(cmdlist, mesh_);
        }
        this->m_renderer->add_lights(cmdlist, this->m_lights);
        this->m_renderer->close_command_list(cmdlist, factory.get_vertex_attributes(), this->m_object_factory);

        // Find the "main" camera if so marked. Otherwise just use the first camera we find.
        std::vector<entity> cameras = this->m_world->get_cameras();
        std::optional<entity> main_camera;
        for (entity camera : cameras) {
            const auto& camera_component = camera.get_component<components::camera_component>();
            if (camera_component.primary) {
                main_camera = camera;
                break;
            }
        }
        if (!main_camera && cameras.size() > 0) {
            main_camera = cameras[0];
        }
        if (main_camera) {
            glm::vec2 size = glm::vec2(this->m_window->get_framebuffer_size());
            this->m_renderer->set_camera_data(*main_camera, size.x / size.y);
        }

        this->m_renderer->render(cmdlist, this->m_shaders["block"], this->m_window->get_context(), this->m_atlas);
        this->m_renderer->destroy_command_list(cmdlist);
        this->m_window->swap_buffers();
    }
    void application::load_assemblies() {
        std::vector<std::filesystem::path> assembly_paths = {
            std::filesystem::current_path() / "BasicVoxelEngine.dll",
            std::filesystem::current_path() / "BasicVoxelEngine.Content.dll",
        };
        for (const auto& path : assembly_paths) {
            this->m_code_host->load_assembly(path);
            spdlog::info("[application] loaded assembly: " + path.string());
        }
        for (const auto& pair : code_host::get_script_wrappers()) {
            this->m_code_host->register_function(pair.first, pair.second);
        }
    }
}