#include "bve_pch.h"
#include "components.h"
#include "application.h"
#include "player.h"
int main(int argc, const char** argv) {
#ifdef NDEBUG
    try {
#endif
    using namespace bve;
    auto& app = application::get();
    app.get_world()->create().add_component<components::script_component>().bind<player>();
    app.run();
    return EXIT_SUCCESS;
#ifdef NDEBUG
    } catch (const std::runtime_error& exc) {
        spdlog::error(exc.what());
        return EXIT_FAILURE;
    }
#endif
}