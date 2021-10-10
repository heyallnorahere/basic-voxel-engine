#pragma once
#include "window.h"
#include "world.h"
#include "renderer.h"
#include "input_manager.h"
#include "texture_atlas.h"
#include "mesh_factory.h"
#include "code_host.h"
#include "graphics/object_factory.h"
namespace bve {
    class application {
    public:
        static application& get();
        ~application();
        application(const application&) = delete;
        application& operator=(const application&) = delete;
        void run();
        void quit();
        double get_delta_time();
        ref<graphics::shader> get_shader(const std::string& name);
        ref<world> get_world() { return this->m_world; }
        ref<renderer> get_renderer() { return this->m_renderer; }
        ref<input_manager> get_input_manager() { return this->m_input_manager; }
        ref<window> get_window() { return this->m_window; }
        ref<texture_atlas> get_texture_atlas() { return this->m_atlas; }
        ref<graphics::object_factory> get_object_factory() { return this->m_object_factory; }
        ref<code_host> get_code_host() { return this->m_code_host; }
    private:
        application();
        void update();
        void render();
        void load_assemblies();
        ref<world> m_world;
        ref<renderer> m_renderer;
        ref<input_manager> m_input_manager;
        ref<window> m_window;
        ref<texture_atlas> m_atlas;
        ref<graphics::object_factory> m_object_factory;
        ref<code_host> m_code_host;
        std::unordered_map<std::string, ref<graphics::shader>> m_shaders;
        std::vector<std::pair<glm::vec3, ref<lighting::light>>> m_lights;
        std::vector<ref<mesh>> m_meshes;
        bool m_running;
        double m_delta_time, m_last_frame;
    };
}