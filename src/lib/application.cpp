#include "bve_pch.h"
#include "application.h"
#include "asset_manager.h"
#include "components.h"
#include "shader_compiler.h"
namespace bve {
    static graphics::graphics_api get_graphics_api() {
        static std::unordered_map<std::string, graphics::graphics_api> apis = {
            { "VULKAN", graphics::graphics_api::VULKAN },
            { "OPENGL", graphics::graphics_api::OPENGL }
        };
        std::string api_name = BVE_GRAPHICS_API;
        for (size_t i = 0; i < api_name.length(); i++) {
            api_name[i] = toupper(api_name[i]);
        }
        return apis[api_name];
    }
    application& application::get() {
        static application instance;
        return instance;
    }
    application::~application() {
        code_host::remove_current();
        shader_compiler::cleanup_compiler();
    }
    void application::run() {
        {
            auto contentloader_class = this->m_code_host->find_class("BasicVoxelEngine.ContentLoading.ContentLoader");
            auto loadregisteredobjects = contentloader_class->get_method("*:LoadRegisteredObjects");
            managed::class_::invoke(loadregisteredobjects);
        }
        auto on_block_changed = [this](glm::ivec3, ref<world> world_) {
            mesh_factory factory(world_);
            this->m_meshes.clear();
            auto clusters = factory.get_clusters(this->m_lights);
            for (const auto& cluster : clusters) {
                this->m_meshes.push_back(factory.create_mesh(cluster));
            }
        };
        on_block_changed(glm::ivec3(0), this->m_world);
        this->m_world->on_block_changed(on_block_changed);
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
        spdlog::info("[application] starting BVE, working directory: {0}", fs::current_path().string());
        shader_compiler::initialize_compiler();
        this->m_code_host = ref<code_host>::create();
        this->load_assemblies();
        {
            auto app_class = this->m_code_host->find_class("BasicVoxelEngine.ContentLoading.ContentLoader");
            auto load_content = app_class->get_method("*:LoadContent");
            managed::class_::invoke(load_content);
        }
        this->m_object_factory = graphics::object_factory::create(get_graphics_api());
        asset_manager& asset_manager_ = asset_manager::get();
        asset_manager_.reload({ fs::current_path() / "assets" });
        this->m_world = ref<world>::create();
        this->m_world->generate();
        this->m_window = ref<window>::create(1600, 900, this->m_object_factory->create_context());
        this->m_atlas = asset_manager_.create_texture_atlas(this->m_object_factory);
        this->m_shaders["static"] = this->m_object_factory->create_shader({ asset_manager_.get_asset_path("shaders:static.glsl") }); // todo: revert back to hlsl
        this->m_renderer = ref<renderer>::create(this->m_object_factory);
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
        this->m_renderer->set_shader(this->m_shaders["static"]);
        auto cmdlist = this->m_renderer->create_command_list();
        for (auto& mesh_ : this->m_meshes) {
            this->m_renderer->add_mesh(cmdlist, mesh_);
        }
        this->m_renderer->add_lights(cmdlist, this->m_lights);
        this->m_renderer->close_command_list(cmdlist, mesh_factory::get_vertex_attributes());

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
        this->m_window->get_context()->clear(glm::vec4(0.f, 0.f, 0.f, 1.f));
        this->m_renderer->render(cmdlist, this->m_window->get_context(), this->m_atlas);
        this->m_window->swap_buffers();
        this->m_renderer->destroy_command_list(cmdlist);
    }
    void application::load_assemblies() {
        std::vector<fs::path> assembly_paths = {
            fs::current_path() / "BasicVoxelEngine.dll",
            fs::current_path() / "BasicVoxelEngine.Content.dll",
        };
        auto mods_dir = std::filesystem::current_path() / "mods";
        if (std::filesystem::is_directory(mods_dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(mods_dir)) {
                const auto& path = entry.path();
                if (path.extension() == ".dll") {
                    assembly_paths.push_back(path);
                }
            }
        }
        for (const auto& path : assembly_paths) {
            this->m_code_host->load_assembly(path);
            spdlog::info("[application] loaded assembly: " + path.string());
        }
        for (const auto& pair : code_host::get_script_wrappers()) {
            this->m_code_host->register_function(pair.first, pair.second);
        }
    }
}