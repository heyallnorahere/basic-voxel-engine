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

        void BasicVoxelEngine_Registry_RegisterTypes() {
            // todo: register managed to native type pairs
        }
        bool BasicVoxelEngine_Registry_RegisterExists(Type type) {
            // todo: check if the specified managed type has a native counterpart
            return false;
        }
        IntPtr BasicVoxelEngine_Registry_CreateRegisterRef(Type type) {
            // todo: create a ref<> of the specified register and return the pointer; it will be deleted via RegisteredObject<>
            return nullptr;
        }

        IntPtr BasicVoxelEngine_Register_CreateRef(int32_t index, Type type, IntPtr address) {
            // todo: create a ref<> of the specified element
            return nullptr;
        }
        int32_t BasicVoxelEngine_Register_GetCount(Type type, IntPtr address) {
            // todo: get the size of the specified register
            return 0;
        }
        MonoObject* BasicVoxelEngine_Register_GetIndex(NamespacedName namespacedName, Type type, IntPtr address) {
            // todo: get the index of the specified register name; if it exists, box it in an object; if not, return nullptr
            return nullptr;
        }
        MonoObject* BasicVoxelEngine_Register_GetNamespacedName(int32_t index, Type type, IntPtr address) {
            // todo: the above but inverse
            return nullptr;
        }
        int32_t BasicVoxelEngine_Register_RegisterObject(MonoObject* object, NamespacedName namespacedName, Type type, IntPtr address) {
            // todo: register the object in the specified register
            return 0;
        }
        bool BasicVoxelEngine_Register_IsManaged(int32_t index, Type type, IntPtr address) {
            // todo: check if the specified element is managed
            return false;
        }
        MonoObject* BasicVoxelEngine_Register_GetManagedObject(int32_t index, Type type, IntPtr address) {
            // todo: get the managed object in the specified element
            return nullptr;
        }

        void BasicVoxelEngine_RegisteredObject_DestroyRef(IntPtr nativeAddress, Type type) {
            // todo: delete the passed ref<>
        }

        float BasicVoxelEngine_Block_GetOpacity(IntPtr nativeAddress) {
            // todo: get the passed block's opacity
            return 0.f;
        }
        bool BasicVoxelEngine_Block_GetSolid(IntPtr nativeAddress) {
            // todo: check if the passed block is solid
            return false;
        }
        string BasicVoxelEngine_Block_GetFriendlyName(IntPtr nativeAddress) {
            // todo: get the passed block's name
            return nullptr;
        }
    }
}