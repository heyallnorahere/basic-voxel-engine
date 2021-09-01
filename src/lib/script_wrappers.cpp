#include "bve_pch.h"
#include "script_wrappers.h"
#include "application.h"
#include "code_host.h"
#include "block.h"
namespace bve {
    namespace script_wrappers {
        class managed_block : public block {
        public:
            managed_block(ref<managed::object> object) {
                this->m_object = object;
            }
            ref<managed::object> get_object() {
                return this->m_object;
            }
            virtual float opacity() override {
                auto property = this->get_property("Opacity");
                return this->m_object->get(property)->unbox<float>();
            }
            virtual bool solid() override {
                auto property = this->get_property("Solid");
                return this->m_object->get(property)->unbox<bool>();
            }
            // todo: get_light() and get_model()
            virtual std::string friendly_name() override {
                auto property = this->get_property("FriendlyName");
                return this->m_object->get(property)->get_string();
            }
            virtual bool managed() override {
                return true;
            }
        private:
            ref<managed::object> m_object;
            ref<managed::class_> get_class() {
                ref<code_host> host = code_host::current();
                return host->find_class("BasicVoxelEngine.Block");
            }
            MonoProperty* get_property(const std::string& name) {
                auto _class = this->get_class();
                return _class->get_property(name);
            }
        };
        static namespaced_name get_native(const NamespacedName& managed_object) {
            MonoDomain* domain = mono_domain_get();
            std::string namespace_name = ref<managed::object>::create((MonoObject*)managed_object.namespace_name, domain)->get_string();
            std::string local_name = ref<managed::object>::create((MonoObject*)managed_object.local_name, domain)->get_string();
            return namespaced_name(namespace_name, local_name);
        }
        static NamespacedName get_managed(const namespaced_name& name) {
            MonoDomain* domain = mono_domain_get();
            MonoString* namespace_name = mono_string_new(domain, name.namespace_name.c_str());
            MonoString* local_name = mono_string_new(domain, name.local_name.c_str());
            return { namespace_name, local_name };
        }
        static std::string get_string(MonoString* string) {
            MonoDomain* current = mono_domain_get();
            auto object = ref<managed::object>::create((MonoObject*)string, current);
            return object->get_string();
        }
        static std::string get_log_message(MonoString* string) {
            return "[managed log] " + get_string(string);
        }

        template<typename T> struct managed_object_ref_hasher {
            using ref_type = ref<T>;
            size_t operator()(ref_type to_hash) const {
                std::hash<void*> hasher;
                return hasher(to_hash->get());
            }
        };
        static void no_implementation_exists() {
            throw std::runtime_error("[script wrappers] no specific implementation exists for this type yet");
        }
        template<typename T> static object_register<T>& get_register(void* ref_address) {
            return **(ref<object_register<T>>*)ref_address;
        }
        template<typename T> static void* create_register_ref() {
            return new ref<object_register<T>>(&registry::get().get_register<T>());
        }
        template<typename T> static void* create_ref(size_t index, void* ref_address) {
            auto& register_ = get_register<T>(ref_address);
            return new ref<T>(register_[index]);
        }
        template<typename T> static size_t get_register_size(void* ref_address) {
            auto& register_ = get_register<T>(ref_address);
            return register_.size();
        }
        template<typename T> static std::optional<size_t> get_index(const namespaced_name& name, void* ref_address) {
            auto& register_ = get_register<T>(ref_address);
            return register_.get_index(name);
        }
        template<typename T> static std::optional<namespaced_name> get_name(size_t index, void* ref_address) {
            auto& register_ = get_register<T>(ref_address);
            return register_.get_name(index);
        }
        template<typename T> static size_t register_object(ref<managed::object>, const namespaced_name&, void*) {
            no_implementation_exists();
            return 0;
        }
        template<> size_t register_object<block>(ref<managed::object> object, const namespaced_name& name, void* ref_address) {
            auto& block_register = get_register<block>(ref_address);
            size_t index = block_register.size();
            block_register.add(ref<managed_block>::create(object), name);
            return index;
        }
        template<typename T> static bool is_managed(size_t, void*) {
            no_implementation_exists();
            return false;
        }
        template<> bool is_managed<block>(size_t index, void* ref_address) {
            auto& block_register = get_register<block>(ref_address);
            return block_register[index]->managed();
        }
        template<typename T> static ref<managed::object> get_managed_object(size_t, void*) {
            no_implementation_exists();
            return nullptr;
        }
        template<> ref<managed::object> get_managed_object<block>(size_t index, void* ref_address) {
            auto& block_register = get_register<block>(ref_address);
            ref<managed_block> block_ = block_register[index];
            return block_->get_object();
        }
        template<typename T> static void delete_ref(void* pointer) {
            delete (ref<T>*)pointer;
        }
        struct registry_callbacks_t {
            std::function<void*()> create_register_ref_callback;
            std::function<void*(size_t, void*)> create_ref_callback;
            std::function<size_t(void*)> get_register_size_callback;
            std::function<std::optional<size_t>(const namespaced_name&, void*)> get_index_callback;
            std::function<std::optional<namespaced_name>(size_t, void*)> get_name_callback;
            std::function<size_t(ref<managed::object>, const namespaced_name&, void*)> register_object_callback;
            std::function<bool(size_t, void*)> is_managed_callback;
            std::function<ref<managed::object>(size_t, void*)> get_managed_object_callback;
        };
        static std::unordered_map<void*, registry_callbacks_t> registry_callbacks;
        static std::unordered_map<void*, std::function<void(void*)>> ref_destruction_callbacks;
        static ref<managed::type> get_type(const std::string& full_name) {
            ref<code_host> current_host = code_host::current();
            ref<managed::class_> found_class = current_host->find_class(full_name);
            if (!found_class) {
                throw std::runtime_error("[script wrappers] could not find type: " + full_name);
            }
            return managed::type::get_type(found_class);
        }
        static ref<managed::type> get_type(MonoReflectionType* reflection_type) {
            MonoType* type = mono_reflection_type_get_type(reflection_type);
            MonoDomain* domain = mono_domain_get();
            return ref<managed::type>::create(type, domain);
        }
        static ref<managed::type> get_register_type(ref<managed::type> element_type) {
            ref<code_host> host = code_host::current();
            MonoDomain* domain = host->get_domain();
            MonoImage* mscorlib_image = mono_get_corlib();
            MonoClass* type_class = mono_class_from_name(mscorlib_image, "System", "Type");
            MonoArray* type_array = mono_array_new(domain, type_class, 1);
            uint32_t array_handle = mono_gchandle_new((MonoObject*)type_array, true);
            mono_array_set(type_array, MonoReflectionType*, 0, element_type->get_object());
            auto register_type = managed::type::get_type(host->find_class("BasicVoxelEngine.Register`1"));
            auto helper_class = host->find_class("BasicVoxelEngine.Helpers");
            MonoMethod* get_generic_type = helper_class->get_method("BasicVoxelEngine.Helpers:GetGenericType(Type,Type[])");
            auto generic_type = (MonoReflectionType*)helper_class->invoke(get_generic_type, register_type->get_object(), type_array)->get();
            mono_gchandle_free(array_handle);
            return ref<managed::type>::create(generic_type, domain);
        }
        template<typename T> static void register_managed_type(const std::string& full_name) {
            ref<managed::type> type = get_type(full_name);
            registry_callbacks[type->get()] = registry_callbacks_t{
                create_register_ref<T>,
                create_ref<T>,
                get_register_size<T>,
                get_index<T>,
                get_name<T>,
                register_object<T>,
                is_managed<T>,
                get_managed_object<T>,
            };
            ref_destruction_callbacks[type->get()] = delete_ref<T>;
            ref_destruction_callbacks[get_register_type(type)->get()] = delete_ref<object_register<T>>;
        }

        double BasicVoxelEngine_Application_GetDeltaTime() {
            auto& app = application::get();
            return app.get_delta_time();
        }

        void BasicVoxelEngine_Logger_PrintDebug(string message) {
            spdlog::debug(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintInfo(string message) {
            spdlog::info(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintWarning(MonoString* message) {
            spdlog::warn(get_log_message(message));
        }
        void BasicVoxelEngine_Logger_PrintError(MonoString* message) {
            spdlog::error(get_log_message(message));
        }

        void BasicVoxelEngine_Registry_RegisterTypes() {
            register_managed_type<block>("BasicVoxelEngine.Block");
        }
        bool BasicVoxelEngine_Registry_RegisterExists(Type type) {
            // all this does is check if the specified has a native counterpart, not if the register actually exists
            ref<managed::type> type_ = get_type(type);
            return registry_callbacks.find(type_->get()) != registry_callbacks.end();
        }
        IntPtr BasicVoxelEngine_Registry_CreateRegisterRef(Type type) {
            return registry_callbacks[get_type(type)->get()].create_register_ref_callback();
        }

        IntPtr BasicVoxelEngine_Register_CreateRef(int32_t index, Type type, IntPtr address) {
            return registry_callbacks[get_type(type)->get()].create_ref_callback((size_t)index, address);
        }
        int32_t BasicVoxelEngine_Register_GetCount(Type type, IntPtr address) {
            auto callback = registry_callbacks[get_type(type)->get()].get_register_size_callback;
            return (int32_t)callback(address);
        }
        bool BasicVoxelEngine_Register_GetIndex(NamespacedName namespacedName, Type type, IntPtr address, int32_t* index) {
            namespaced_name native_name = get_native(namespacedName);
            auto optional = registry_callbacks[get_type(type)->get()].get_index_callback(native_name, address);
            if (optional) {
                *index = (int32_t)*optional;
                return true;
            } else {
                return false;
            }
        }
        bool BasicVoxelEngine_Register_GetNamespacedName(int32_t index, Type type, IntPtr address, NamespacedName* namespacedName) {
            auto optional = registry_callbacks[get_type(type)->get()].get_name_callback((size_t)index, address);
            if (optional) {
                *namespacedName = get_managed(*optional);
                return true;
            } else {
                return false;
            }
        }
        int32_t BasicVoxelEngine_Register_RegisterObject(MonoObject* object, NamespacedName namespacedName, Type type, IntPtr address) {
            MonoDomain* domain = mono_domain_get();
            auto callback = registry_callbacks[get_type(type)->get()].register_object_callback;
            auto managed_object = ref<managed::object>::create(object, domain);
            auto name = get_native(namespacedName);
            return (int32_t)callback(managed_object, name, address);
        }
        bool BasicVoxelEngine_Register_IsManaged(int32_t index, Type type, IntPtr address) {
            auto callback = registry_callbacks[get_type(type)->get()].is_managed_callback;
            return callback((size_t)index, address);
        }
        MonoObject* BasicVoxelEngine_Register_GetManagedObject(int32_t index, Type type, IntPtr address) {
            auto callback = registry_callbacks[get_type(type)->get()].get_managed_object_callback;
            return (MonoObject*)callback((size_t)index, address)->get();
        }

        void BasicVoxelEngine_RegisteredObject_DestroyRef(IntPtr nativeAddress, Type type) {
            auto callback = ref_destruction_callbacks[get_type(type)->get()];
            if (callback) {
                callback(nativeAddress);
            }
        }

        float BasicVoxelEngine_Block_GetOpacity(IntPtr nativeAddress) {
            ref<block> block_ = *(ref<block>*)nativeAddress;
            return block_->opacity();
        }
        bool BasicVoxelEngine_Block_GetSolid(IntPtr nativeAddress) {
            ref<block> block_ = *(ref<block>*)nativeAddress;
            return block_->solid();
        }
        string BasicVoxelEngine_Block_GetFriendlyName(IntPtr nativeAddress) {
            ref<block> block_ = *(ref<block>*)nativeAddress;
            MonoDomain* domain = mono_domain_get();
            std::string friendly_name = block_->friendly_name();
            return mono_string_new(domain, friendly_name.c_str());
        }
    }
}