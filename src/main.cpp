#include "bve_pch.h"
#include "window.h"
using callback = std::function<void()>;
static void update() {
    // todo: update
}
static void render(callback clear, callback swap_buffers) {
    clear();
    // todo: render
    swap_buffers();
}
static void main_loop(std::shared_ptr<bve::window> window) {
    callback clear = [window]() { window->clear(); };
    callback swap_buffers = [window]() { window->swap_buffers(); };
    while (!window->should_close()) {
        update();
        render(clear, swap_buffers);
        bve::window::poll_events();
    }
}
int main(int argc, const char** argv) {
    try {
        auto window = std::make_shared<bve::window>(800, 600);
        main_loop(window);
        return EXIT_SUCCESS;
    } catch (std::runtime_error exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
}