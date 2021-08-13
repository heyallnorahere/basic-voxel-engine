#pragma once
#include "window.h"
#include "world.h"
#include "renderer.h"
#include "input_manager.h"
#include "shader.h"
#include "texture_atlas.h"
#include "mesh_factory.h"
namespace bve {
    class application {
    public:
        static application& get();
        application(const application&) = delete;
        application& operator=(const application&) = delete;
        void run();
        void quit();
        double get_delta_time();
        ref<shader> get_shader(const std::string& name);
    private:
        application();
        void update();
        void render();
        ref<world> m_world;
        ref<renderer> m_renderer;
        ref<input_manager> m_input_manager;
        ref<window> m_window;
        ref<texture_atlas> m_atlas;
        std::unordered_map<std::string, ref<shader>> m_shaders;
        std::vector<std::vector<mesh_factory::processed_voxel>> m_clusters;
        bool m_running;
        double m_delta_time, m_last_frame;
    };
}