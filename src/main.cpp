#include "bve_pch.h"
#include "window.h"
#include "world.h"
#include "block.h"
#include "shader.h"
#include "mesh_factory.h"
#include "renderer.h"
namespace bve {
    using callback = std::function<void()>;
    static void update(std::shared_ptr<world> world_) {
        // todo: update
    }
    static void render(callback clear, callback swap_buffers, std::shared_ptr<world> world_, std::shared_ptr<renderer> renderer_, std::shared_ptr<shader> shader_) {
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
        // todo: set camera data
        renderer_->render(cmdlist, shader_);
        renderer_->destroy_command_list(cmdlist);
        swap_buffers();
    }
    static void main_loop(std::shared_ptr<window> window_, std::shared_ptr<world> world_) {
        callback clear = [window_]() { window_->clear(); };
        callback swap_buffers = [window_]() { window_->swap_buffers(); };
        auto shader_ = shader::create({ { "assets/shaders/vertex.glsl", GL_VERTEX_SHADER }, { "assets/shaders/fragment.glsl", GL_FRAGMENT_SHADER } });
        auto renderer_ = std::make_shared<renderer>();
        while (!window_->should_close()) {
            window_->new_frame();
            update(world_);
            render(clear, swap_buffers, world_, renderer_, shader_);
            bve::window::poll_events();
        }
    }
}
int main(int argc, const char** argv) {
    try {
        using namespace bve;
        auto window_ = std::make_shared<window>(800, 600);
        auto world_ = std::make_shared<world>(glm::ivec3(16, 256, 16));
        block::register_all();
        main_loop(window_, world_);
        return EXIT_SUCCESS;
    } catch (std::runtime_error exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
}