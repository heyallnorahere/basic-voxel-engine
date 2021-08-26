#include "bve_pch.h"
#include "script_wrappers.h"
#include "application.h"
#include "code_host.h"
namespace bve {
    namespace script_wrappers {
        static std::string get_string(MonoString* string) {
            MonoDomain* current = mono_domain_get();
            auto object = ref<managed::object>::create((MonoObject*)string, current);
            return object->get_string();
        }
        static std::string get_log_message(MonoString* string) {
            return "[managed log] " + get_string(string);
        }
        double BasicVoxelEngine_Application_GetDeltaTime() {
            auto& app = application::get();
            return app.get_delta_time();
        }
        void BasicVoxelEngine_Logger_PrintDebug(MonoString* message) {
            spdlog::debug(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintInfo(MonoString* message) {
            spdlog::info(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintWarning(MonoString* message) {
            spdlog::warn(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintError(MonoString* message) {
            spdlog::error(get_log_message(message));
        }
    }
}