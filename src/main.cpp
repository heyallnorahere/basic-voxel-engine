#include "bve_pch.h"
#include "world.h"
#include "window.h"
int main(int argc, const char** argv) {
    try {
        auto window = std::make_shared<bve::window>(800, 600);
        return EXIT_SUCCESS;
    } catch (std::runtime_error exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
}