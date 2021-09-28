#include "bve_pch.h"
#include "script_wrappers.h"
#include "application.h"
#include "code_host.h"
#include "block.h"
#include "asset_manager.h"
#include "lighting/spotlight.h"
#include "lighting/point_light.h"
#include "components.h"
namespace bve {
    namespace script_wrappers {
        struct script_ref {
            void* data;
            std::function<void(void*)> cleanup;
            ~script_ref() {
                cleanup(data);
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
        class managed_block : public block {
        public:
            managed_block(ref<managed::object> object) {
                this->m_object = object;
            }
            ref<managed::object> get_object() {
                return this->m_object;
            }
            virtual void load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name) override {
                auto _class = this->get_class();
                auto load = _class->get_method("*:Load");
                if (!load) {
                    return;
                }
                NamespacedName managed_register_name = get_managed(register_name);
                ref<managed::object> factory = this->create_factory_object(object_factory);
                this->m_object->invoke(load, factory->get(), &managed_register_name);
            }
            virtual float opacity() override {
                auto property = this->get_property("Opacity");
                return this->m_object->get(property)->unbox<float>();
            }
            virtual bool solid() override {
                auto property = this->get_property("Solid");
                return this->m_object->get(property)->unbox<bool>();
            }
            virtual ref<model> get_model() override {
                auto property = this->get_property("Model");
                return this->get_model(this->m_object->get(property));
            }
            virtual ref<lighting::light> get_light() override {
                auto property = this->get_property("Light");
                return this->get_light(this->m_object->get(property));
            }
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
                return managed::class_::get_class(this->m_object);
            }
            MonoProperty* get_property(const std::string& name) {
                auto _class = this->get_class();
                return _class->get_property(name);
            }
            ref<managed::object> create_factory_object(ref<graphics::object_factory> factory) {
                ref<code_host> host = code_host::current();
                ref<managed::class_> factory_class = host->find_class("BasicVoxelEngine.Graphics.Factory");
                auto pointer = new ref<graphics::object_factory>(factory);
                return factory_class->instantiate(&pointer);
            }
            ref<model> get_model(ref<managed::object> object) {
                if (!object || !object->get()) {
                    return nullptr;
                }
                // very hacky but i had no other choice
                ref<code_host> host = code_host::current();
                ref<managed::class_> model_class = host->find_class("BasicVoxelEngine.Model");
                auto field = model_class->get_field("mNativeAddress");
                void* pointer = object->get(field)->unbox<void*>();
                return *(ref<model>*)pointer;
            }
            ref<lighting::light> get_light(ref<managed::object> object) {
                if (!object || !object->get()) {
                    return nullptr;
                }
                ref<code_host> host = code_host::current();
                ref<managed::class_> light_class = host->find_class("BasicVoxelEngine.Lighting.Light");
                auto field = light_class->get_field("mAddress");
                void* pointer = object->get(field)->unbox<void*>();
                return *(ref<lighting::light>*)pointer;
            }
        };
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
            auto& ref_struct = *(script_ref*)ref_address;
            return **(ref<object_register<T>>*)ref_struct.data;
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

        void BasicVoxelEngine_Application_Quit() {
            auto& app = application::get();
            app.quit();
        }
        double BasicVoxelEngine_Application_GetDeltaTime() {
            auto& app = application::get();
            return app.get_delta_time();
        }
        IntPtr BasicVoxelEngine_Application_GetWorld() {
            auto& app = application::get();
            return new ref<world>(app.get_world());
        }
        IntPtr BasicVoxelEngine_Application_GetInputManager() {
            auto& app = application::get();
            return new ref<input_manager>(app.get_input_manager());
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
            ref<managed::type> type_object = get_type(type);
            void* ref_pointer = registry_callbacks[type_object->get()].create_register_ref_callback();
            ref<managed::type> register_type = get_register_type(type_object);
            auto cleanup = ref_destruction_callbacks[register_type->get()];
            return new script_ref{ ref_pointer, cleanup };
        }
        IntPtr BasicVoxelEngine_Register_CreateRef(int32_t index, Type type, IntPtr address) {
            ref<managed::type> type_object = get_type(type);
            void* ref_pointer = registry_callbacks[type_object->get()].create_ref_callback((size_t)index, address);
            auto cleanup = ref_destruction_callbacks[type_object->get()];
            return new script_ref{ ref_pointer, cleanup };
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

        void BasicVoxelEngine_RegisteredObject_DestroyRef(IntPtr nativeAddress) {
            delete (script_ref*)nativeAddress;
        }

        static ref<block> get_block_ref(void* address) {
            auto& ref_struct = *(script_ref*)address;
            return *(ref<block>*)ref_struct.data;
        }
        float BasicVoxelEngine_Block_GetOpacity(IntPtr nativeAddress) {
            ref<block> block_ = get_block_ref(nativeAddress);
            return block_->opacity();
        }
        bool BasicVoxelEngine_Block_GetSolid(IntPtr nativeAddress) {
            ref<block> block_ = get_block_ref(nativeAddress);
            return block_->solid();
        }
        string BasicVoxelEngine_Block_GetFriendlyName(IntPtr nativeAddress) {
            ref<block> block_ = get_block_ref(nativeAddress);
            MonoDomain* domain = mono_domain_get();
            std::string friendly_name = block_->friendly_name();
            return mono_string_new(domain, friendly_name.c_str());
        }
        IntPtr BasicVoxelEngine_Block_GetModel(IntPtr nativeAddress) {
            ref<block> block_ = get_block_ref(nativeAddress);
            return new ref<model>(block_->get_model());
        }
        IntPtr BasicVoxelEngine_Block_GetLight(IntPtr nativeAddress, LightType* type) {
            ref<block> block_ = get_block_ref(nativeAddress);
            auto light = block_->get_light();
            if (light) {
                *type = light->get_type();
                return new ref<lighting::light>(light);
            } else {
                return nullptr;
            }
        }

        void BasicVoxelEngine_Graphics_Factory_DestroyRef(IntPtr address) {
            delete (ref<graphics::object_factory>*)address;
        }

        IntPtr BasicVoxelEngine_Model_LoadModel(string path, IntPtr factory) {
            std::filesystem::path native_path = get_string(path);
            auto factory_ref = *(ref<graphics::object_factory>*)factory;
            return new ref<model>(new model(native_path, factory_ref));
        }
        void BasicVoxelEngine_Model_DestroyRef(IntPtr address) {
            delete (ref<model>*)address;
        }

        string BasicVoxelEngine_AssetManager_GetAssetPath(string assetName) {
            auto& asset_manager_ = asset_manager::get();
            auto path = asset_manager_.get_asset_path(get_string(assetName)).string();
            return mono_string_new(mono_domain_get(), path.c_str());
        }

        ref<lighting::light> get_light_ref(void* address) {
            return *(ref<lighting::light>*)address;
        }
        void BasicVoxelEngine_Lighting_Light_Destroy(IntPtr address) {
            delete (ref<lighting::light>*)address;
        }
        void BasicVoxelEngine_Lighting_Light_SetColor(IntPtr address, Vector3 color) {
            auto light = get_light_ref(address);
            light->set_color(color);
        }
        void BasicVoxelEngine_Lighting_Light_SetAmbientStrength(IntPtr address, float strength) {
            auto light = get_light_ref(address);
            light->set_ambient_strength(strength);
        }
        void BasicVoxelEngine_Lighting_Light_SetSpecularStrength(IntPtr address, float strength) {
            auto light = get_light_ref(address);
            light->set_specular_strength(strength);
        }
        LightType BasicVoxelEngine_Lighting_Light_GetType(IntPtr address) {
            auto light = get_light_ref(address);
            return light->get_type();
        }

        ref<lighting::spotlight> get_spotlight_ref(void* address) {
            return *(ref<lighting::light>*)address;
        }
        IntPtr BasicVoxelEngine_Lighting_Spotlight_Create() {
            return new ref<lighting::light>(new lighting::spotlight);
        }
        void BasicVoxelEngine_Lighting_Spotlight_SetDirection(IntPtr address, Vector3 direction) {
            auto light = get_spotlight_ref(address);
            light->set_direction(direction);
        }
        void BasicVoxelEngine_Lighting_Spotlight_SetCutoff(IntPtr address, float cutoff) {
            auto light = get_spotlight_ref(address);
            light->set_cutoff(cutoff);
        }

        ref<lighting::point_light> get_point_light_ref(void* address) {
            return *(ref<lighting::light>*)address;
        }
        IntPtr BasicVoxelEngine_Lighting_PointLight_Create() {
            return new ref<lighting::light>(new lighting::point_light);
        }
        void BasicVoxelEngine_Lighting_PointLight_SetConstant(IntPtr address, float constant) {
            auto light = get_point_light_ref(address);
            light->set_constant(constant);
        }
        void BasicVoxelEngine_Lighting_PointLight_SetLinear(IntPtr address, float linear) {
            auto light = get_point_light_ref(address);
            light->set_linear(linear);
        }
        void BasicVoxelEngine_Lighting_PointLight_SetQuadratic(IntPtr address, float quadratic) {
            auto light = get_point_light_ref(address);
            light->set_quadratic(quadratic);
        }
        
        ref<world> get_world_ref(void* address) {
            return *(ref<world>*)address;
        }
        void BasicVoxelEngine_World_Destroy(IntPtr address) {
            delete (ref<world>*)address;
        }
        int32_t BasicVoxelEngine_World_GetBlock(IntPtr address, Vector3I position) {
            auto _world = get_world_ref(address);
            size_t block_type;
            _world->get_block(position, block_type);
            return (int32_t)block_type;
        }
        void BasicVoxelEngine_World_SetBlock(IntPtr address, Vector3I position, int32_t index) {
            auto _world = get_world_ref(address);
            _world->set_block(position, (size_t)index);
        }
        void BasicVoxelEngine_World_AddOnBlockChangedCallback(IntPtr address, MonoObject* callback) {
            auto _world = get_world_ref(address);
            auto delegate_ref = ref<managed::delegate>::create(callback, mono_domain_get());
            _world->on_block_changed([delegate_ref](glm::ivec3 position, ref<world> world_) {
                auto host = code_host::current();
                auto world_class = host->find_class("BasicVoxelEngine.World");
                auto instance = world_class->instantiate(new ref<world>(world_));
                ref<managed::delegate> delegate_copy = delegate_ref;
                delegate_copy->invoke(&position, instance->get());
            });
        }

        template<typename T> void* add_component(entity ent) {
            return &ent.add_component<T>();
        }
        template<typename T> void* get_component(entity ent) {
            return &ent.get_component<T>();
        }
        template<typename T> bool has_component(entity ent) {
            return ent.has_component<T>();
        }
        template<typename T> void remove_component(entity ent) {
            ent.remove_component<T>();
        }
        struct managed_component_data {
            std::function<void*(entity)> add_component, get_component;
            std::function<bool(entity)> has_component;
            std::function<void(entity)> remove_component;
            std::function<ref<managed::object>(void*)> create_object;
        };
        std::unordered_map<MonoReflectionType*, managed_component_data> component_data;
        template<typename T> static void register_component_type(ref<code_host> host, const std::string& type_name) {
            auto _class = host->find_class(type_name);
            auto type = managed::type::get_type(_class)->get_object();
            std::function<ref<managed::object>(void*)> create_object = [_class](void* pointer) mutable {
                return _class->instantiate(&pointer);
            };
            component_data.insert({ type, {
                add_component<T>,
                get_component<T>,
                has_component<T>,
                remove_component<T>,
                create_object
            } });
        }
        static entity get_entity(uint32_t id, void* world_) {
            ref<world> _ref = *(ref<world>*)world_;
            return entity((entt::entity)id, _ref.raw());
        }
        object BasicVoxelEngine_Entity_AddComponent(uint id, IntPtr world, Type type) {
            entity ent = get_entity(id, world);
            const auto& data = component_data[type];
            void* pointer = data.add_component(ent);
            return (MonoObject*)data.create_object(pointer)->get();
        }
        object BasicVoxelEngine_Entity_GetComponent(uint id, IntPtr world, Type type) {
            entity ent = get_entity(id, world);
            const auto& data = component_data[type];
            void* pointer = data.get_component(ent);
            return (MonoObject*)data.create_object(pointer)->get();
        }
        bool BasicVoxelEngine_Entity_HasComponent(uint id, IntPtr world, Type type) {
            entity ent = get_entity(id, world);
            const auto& data = component_data[type];
            return data.has_component(ent);
        }
        void BasicVoxelEngine_Entity_RemoveComponent(uint id, IntPtr world, Type type) {
            entity ent = get_entity(id, world);
            const auto& data = component_data[type];
            data.remove_component(ent);
        }
        void BasicVoxelEngine_Entity_RegisterComponents() {
            ref<code_host> host = code_host::current();
            register_component_type<components::transform_component>(host, "BasicVoxelEngine.Components.TransformComponent");
            register_component_type<components::camera_component>(host, "BasicVoxelEngine.Components.CameraComponent");
        }

        Vector3 BasicVoxelEngine_Components_TransformComponent_GetTranslation(IntPtr address) {
            return ((components::transform_component*)address)->translation;
        }
        void BasicVoxelEngine_Components_TransformComponent_SetTranslation(IntPtr address, Vector3 value) {
            ((components::transform_component*)address)->translation = value;
        }
        Vector3 BasicVoxelEngine_Components_TransformComponent_GetRotation(IntPtr address) {
            return ((components::transform_component*)address)->rotation;
        }
        void BasicVoxelEngine_Components_TransformComponent_SetRotation(IntPtr address, Vector3 value) {
            ((components::transform_component*)address)->rotation = value;
        }
        Vector3 BasicVoxelEngine_Components_TransformComponent_GetScale(IntPtr address) {
            return ((components::transform_component*)address)->scale;
        }
        void BasicVoxelEngine_Components_TransformComponent_SetScale(IntPtr address, Vector3 value) {
            ((components::transform_component*)address)->scale = value;
        }

        Vector3 BasicVoxelEngine_Components_CameraComponent_GetDirection(IntPtr address) {
            return ((components::camera_component*)address)->direction;
        }
        void BasicVoxelEngine_Components_CameraComponent_SetDirection(IntPtr address, Vector3 value) {
            ((components::camera_component*)address)->direction = value;
        }
        Vector3 BasicVoxelEngine_Components_CameraComponent_GetUp(IntPtr address) {
            return ((components::camera_component*)address)->up;
        }
        void BasicVoxelEngine_Components_CameraComponent_SetUp(IntPtr address, Vector3 value) {
            ((components::camera_component*)address)->up = value;
        }
        bool BasicVoxelEngine_Components_CameraComponent_GetPrimary(IntPtr address) {
            return ((components::camera_component*)address)->primary;
        }
        void BasicVoxelEngine_Components_CameraComponent_SetPrimary(IntPtr address, bool value) {
            ((components::camera_component*)address)->primary = value;
        }
        float BasicVoxelEngine_Components_CameraComponent_GetNearPlane(IntPtr address) {
            return ((components::camera_component*)address)->near_plane;
        }
        void BasicVoxelEngine_Components_CameraComponent_SetNearPlane(IntPtr address, float value) {
            ((components::camera_component*)address)->near_plane = value;
        }
        float BasicVoxelEngine_Components_CameraComponent_GetFarPlane(IntPtr address) {
            return ((components::camera_component*)address)->far_plane;
        }
        void BasicVoxelEngine_Components_CameraComponent_SetFarPlane(IntPtr address, float value) {
            ((components::camera_component*)address)->far_plane = value;
        }

        static ref<input_manager> get_input_manager(void* address) {
            return *(ref<input_manager>*)address;
        }
        void BasicVoxelEngine_InputManager_DestroyRef(IntPtr address) {
            delete (ref<input_manager>*)address;
        }
        bool BasicVoxelEngine_InputManager_GetMouseEnabled(IntPtr address) {
            auto im = get_input_manager(address);
            return im->mouse_enabled();
        }
        void BasicVoxelEngine_InputManager_SetMouseEnabled(IntPtr address, bool value) {
            auto im = get_input_manager(address);
            im->mouse_enabled() = value;
        }
        Vector2 BasicVoxelEngine_InputManager_GetMouse(IntPtr address) {
            auto im = get_input_manager(address);
            return im->get_mouse();
        }
        input_manager::key_state BasicVoxelEngine_InputManager_GetKey(IntPtr address, int32_t key) {
            auto im = get_input_manager(address);
            return im->get_key(key);
        }
    }
}