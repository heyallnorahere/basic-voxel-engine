#include "bve_pch.h"
#include "script_wrappers.h"
#include "application.h"
#include "code_host.h"
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
        static std::string get_string(MonoString* string) {
            MonoDomain* current = mono_domain_get();
            auto object = ref<managed::object>::create((MonoObject*)string, current);
            return object->get_string();
        }
        static MonoString* get_string(const std::string& string) {
            MonoDomain* current = mono_domain_get();
            return mono_string_new(current, string.c_str());
        }
        static std::string get_log_message(MonoString* string) {
            return "[managed log] " + get_string(string);
        }

        static void no_implementation_exists() {
            throw std::runtime_error("[script wrappers] no specific implementation exists for this type yet");
        }
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
        IntPtr BasicVoxelEngine_Application_GetFactory() {
            auto& app = application::get();
            return new ref<graphics::object_factory>(app.get_object_factory());
        }
        IntPtr BasicVoxelEngine_Application_GetWindow() {
            auto& app = application::get();
            return new ref<window>(app.get_window());
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

        static ref<graphics::object_factory> get_factory_ref(void* address) {
            return *(ref<graphics::object_factory>*)address;
        }
        void BasicVoxelEngine_Graphics_Factory_DestroyRef(IntPtr address) {
            delete (ref<graphics::object_factory>*)address;
        }
        IntPtr BasicVoxelEngine_Graphics_Factory_CreateTexture(IntPtr address, MonoObject* imageData) {
            auto factory = get_factory_ref(address);
            auto data = ref<managed::object>::create(imageData, mono_domain_get());
            auto type = managed::class_::get_class(data);
            MonoProperty* property = type->get_property("Width");
            int32_t width = data->get(property)->unbox<int32_t>();
            property = type->get_property("Height");
            int32_t height = data->get(property)->unbox<int32_t>();
            property = type->get_property("Channels");
            int32_t channels = data->get(property)->unbox<int32_t>();
            MonoMethod* get_byte = type->get_method("*:GetByte");
            std::vector<uint8_t> image_data;
            for (int32_t i = 0; i < width * height * channels; i++) {
                auto return_value = data->invoke(get_byte, &i);
                uint8_t byte = return_value->unbox<uint8_t>();
                image_data.push_back(byte);
            }
            auto texture = factory->create_texture(image_data, width, height, channels);
            return new ref<graphics::texture>(texture);
        }
        IntPtr BasicVoxelEngine_Graphics_Factory_CreateContext(IntPtr address) {
            auto factory = get_factory_ref(address);
            auto context = factory->create_context();
            return new ref<graphics::context>(context);
        }
        IntPtr BasicVoxelEngine_Graphics_Factory_CreateShader(IntPtr address, MonoObject* sourceList) {
            auto factory = get_factory_ref(address);
            std::vector<std::filesystem::path> sources;
            auto source_list = ref<managed::object>::create(sourceList, mono_domain_get());
            auto type = managed::class_::get_class(source_list);
            MonoProperty* property = type->get_property("Count");
            int32_t count = source_list->get(property)->unbox<int32_t>();
            MonoMethod* method = type->get_method("*:GetPath");
            for (int32_t i = 0; i < count; i++) {
                auto path = source_list->invoke(method, &i)->get_string();
                sources.push_back(path);
            }
            auto shader = factory->create_shader(sources);
            return new ref<graphics::shader>(shader);
        }

        IntPtr BasicVoxelEngine_Model_LoadModel(string path, IntPtr factory) {
            fs::path native_path = get_string(path);
            auto factory_ref = *(ref<graphics::object_factory>*)factory;
            return new ref<model>(new model(native_path, factory_ref));
        }
        void BasicVoxelEngine_Model_DestroyRef(IntPtr address) {
            delete (ref<model>*)address;
        }
        static ref<model> get_model_ref(IntPtr address) {
            return *(ref<model>*)address;
        }
        struct mesh_texture_offset_data {
            std::unordered_map<std::string, size_t> data;
            std::vector<std::string> keys;
        };
        int32_t BasicVoxelEngine_Model_GetMeshCount(IntPtr address) {
            auto model_ = get_model_ref(address);
            return (int32_t)model_->get_mesh_count();
        }
        MonoObject* BasicVoxelEngine_Model_GetMesh(IntPtr address, int32_t index, int32_t* vertexOffset, int32_t* indexOffset, int32_t* textureCount) {
            auto model_ = get_model_ref(address);
            auto mesh_data = model_->get_mesh_data((size_t)index);
            *vertexOffset = (int32_t)mesh_data.vertex_offset;
            *indexOffset = (int32_t)mesh_data.index_offset;
            *textureCount = (int32_t)mesh_data.textures.size();
            ref<code_host> host = code_host::current();
            auto offset_data = new mesh_texture_offset_data;
            for (const auto& [key, value] : mesh_data.texture_offsets) {
                offset_data->data.insert({ key, value });
                offset_data->keys.push_back(key);
            }
            auto offset_data_class = host->find_class("BasicVoxelEngine.MeshTextureOffsetData");
            auto return_data = offset_data_class->instantiate(&offset_data);
            return (MonoObject*)return_data->get();
        }
        IntPtr BasicVoxelEngine_Model_GetTexture(IntPtr address, int32_t meshIndex, int32_t textureIndex) {
            auto model_ = get_model_ref(address);
            auto mesh_data = model_->get_mesh_data((size_t)meshIndex);
            auto texture = mesh_data.textures[(size_t)textureIndex];
            return new ref<graphics::texture>(texture);
        }
        int32_t BasicVoxelEngine_Model_GetVertexCount(IntPtr address) {
            auto model_ = get_model_ref(address);
            return (int32_t)model_->get_vertices().size();
        }
        model::vertex BasicVoxelEngine_Model_GetVertex(IntPtr address, int32_t index) {
            auto model_ = get_model_ref(address);
            auto vertices = model_->get_vertices();
            return vertices[(size_t)index];
        }
        int32_t BasicVoxelEngine_Model_GetIndexCount(IntPtr address) {
            auto model_ = get_model_ref(address);
            return (int32_t)model_->get_indices().size();
        }
        uint BasicVoxelEngine_Model_GetIndex(IntPtr address, int32_t index) {
            auto model_ = get_model_ref(address);
            auto indices = model_->get_indices();
            return indices[(size_t)index];
        }

        void BasicVoxelEngine_MeshTextureOffsetData_Destroy(IntPtr address) {
            delete (mesh_texture_offset_data*)address;
        }
        int32_t BasicVoxelEngine_MeshTextureOffsetData_GetCount(IntPtr address) {
            auto data = (mesh_texture_offset_data*)address;
            return (int32_t)data->keys.size();
        }
        string BasicVoxelEngine_MeshTextureOffsetData_GetKey(IntPtr address, int32_t index) {
            auto data = (mesh_texture_offset_data*)address;
            std::string key = data->keys[(size_t)index];
            return get_string(key);
        }
        int32_t BasicVoxelEngine_MeshTextureOffsetData_GetValue(IntPtr address, string key) {
            auto data = (mesh_texture_offset_data*)address;
            std::string key_ = get_string(key);
            return (int32_t)data->data[key_];
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
        void BasicVoxelEngine_InputManager_GetMouse(IntPtr address, Vector2* offset) {
            auto im = get_input_manager(address);
            *offset = im->get_mouse();
        }
        input_manager::key_state BasicVoxelEngine_InputManager_GetKey(IntPtr address, int32_t key) {
            auto im = get_input_manager(address);
            return im->get_key(key);
        }
        input_manager::key_state BasicVoxelEngine_InputManager_GetMouseButton(IntPtr address, int32_t button) {
            auto im = get_input_manager(address);
            return im->get_mouse_button((size_t)button);
        }

        struct image_data_t {
            std::vector<uint8_t> data;
            int32_t width, height, channels;
        };
        IntPtr BasicVoxelEngine_Graphics_ImageData_Load(string path) {
            auto data = new image_data_t;
            std::string path_ = get_string(path);
            if (!graphics::texture::load_image(path_, data->data, data->width, data->height, data->channels)) {
                throw std::runtime_error("[script wrappers] could not load texture: " + path_);
            }
            return data;
        }
        void BasicVoxelEngine_Graphics_ImageData_Destroy(IntPtr address) {
            delete (image_data_t*)address;
        }
        int32_t BasicVoxelEngine_Graphics_ImageData_GetWidth(IntPtr address) {
            auto data = (image_data_t*)address;
            return data->width;
        }
        int32_t BasicVoxelEngine_Graphics_ImageData_GetHeight(IntPtr address) {
            auto data = (image_data_t*)address;
            return data->height;
        }
        int32_t BasicVoxelEngine_Graphics_ImageData_GetChannels(IntPtr address) {
            auto data = (image_data_t*)address;
            return data->channels;
        }
        uint8_t BasicVoxelEngine_Graphics_ImageData_GetByte(IntPtr address, int32_t index) {
            auto data = (image_data_t*)address;
            return data->data[(size_t)index];
        }

        static ref<graphics::texture> get_texture_ref(void* address) {
            return *(ref<graphics::texture>*)address;
        }
        void BasicVoxelEngine_Graphics_Texture_DestroyRef(IntPtr address) {
            delete (ref<graphics::texture>*)address;
        }
        Vector2I BasicVoxelEngine_Graphics_Texture_GetSize(IntPtr address) {
            auto texture = get_texture_ref(address);
            return texture->get_size();
        }
        int32_t BasicVoxelEngine_Graphics_Texture_GetChannels(IntPtr address) {
            auto texture = get_texture_ref(address);
            return texture->get_channels();
        }

        static ref<window> get_window_ref(void* address) {
            return *(ref<window>*)address;
        }
        void BasicVoxelEngine_Window_DestroyRef(IntPtr address) {
            delete (ref<window>*)address;
        }
        IntPtr BasicVoxelEngine_Window_GetContext(IntPtr address) {
            auto window_ = get_window_ref(address);
            return new ref<graphics::context>(window_->get_context());
        }
        Vector2I BasicVoxelEngine_Window_GetFramebufferSize(IntPtr address) {
            auto window_ = get_window_ref(address);
            return window_->get_framebuffer_size();
        }

        static ref<graphics::shader> get_shader_ref(void* address) {
            return *(ref<graphics::shader>*)address;
        }
        void BasicVoxelEngine_Graphics_Shader_DestroyRef(IntPtr address) {
            delete (ref<graphics::shader>*)address;
        }
        void BasicVoxelEngine_Graphics_Shader_Reload(IntPtr address) {
            auto shader = get_shader_ref(address);
            shader->reload();
        }
        void BasicVoxelEngine_Graphics_Shader_Bind(IntPtr address) {
            auto shader = get_shader_ref(address);
            shader->bind();
        }
        void BasicVoxelEngine_Graphics_Shader_Unbind(IntPtr address) {
            auto shader = get_shader_ref(address);
            shader->unbind();
        }
        struct uniform_callbacks_t {
            using set_property_callback_t = std::function<void(void*)>;
            using set_t = std::function<void(ref<graphics::shader>, const std::string&, ref<managed::object>)>;
            using get_t = std::function<void(ref<graphics::shader>, const std::string&, set_property_callback_t)>;
            set_t set;
            get_t get;
        };
        std::unordered_map<MonoReflectionType*, uniform_callbacks_t> uniform_callbacks;
        static void register_uniform_type(const std::string& name, const uniform_callbacks_t::set_t& set, const uniform_callbacks_t::get_t& get) {
            ref<code_host> host = code_host::current();
            auto class_ = host->find_class(name);
            if (!class_ || !class_->get()) {
                return;
            }
            auto type = managed::type::get_type(class_);
            auto key = type->get_object();
            uniform_callbacks.insert({ key, {
                set,
                get
            } });
        }
        void BasicVoxelEngine_Graphics_Shader_Set(IntPtr address, string name, object value, Type type) {
            auto shader = get_shader_ref(address);
            std::string uniform_name = get_string(name);
            auto object = ref<managed::object>::create(value, mono_domain_get());
            if (uniform_callbacks.find(type) == uniform_callbacks.end()) {
                return;
            }
            uniform_callbacks[type].set(shader, uniform_name, object);
        }
        void BasicVoxelEngine_Graphics_Shader_Get(IntPtr address, string name, object value, Type type) {
            auto shader = get_shader_ref(address);
            std::string uniform_name = get_string(name);
            auto object = ref<managed::object>::create(value, mono_domain_get());
            uniform_callbacks_t::set_property_callback_t callback = [object](void* ptr) mutable {
                auto type = managed::class_::get_class(object);
                MonoProperty* property = type->get_property("Data");
                object->set(property, ptr);
            };
            if (uniform_callbacks.find(type) == uniform_callbacks.end()) {
                callback(nullptr);
                return;
            }
            uniform_callbacks[type].get(shader, uniform_name, callback);
        }
        static void set_int(ref<graphics::shader> shader, const std::string& name, ref<managed::object> value) {
            shader->set_int(name, value->unbox<int32_t>());
        }
        static void get_int(ref<graphics::shader> shader, const std::string& name, uniform_callbacks_t::set_property_callback_t callback) {
            int32_t value = shader->get_int(name);
            callback(&value);
        }
        void BasicVoxelEngine_Graphics_Shader_InitializeUniforms() {
            register_uniform_type("System.Int32", set_int, get_int);
            // todo: add more
        }

        static ref<graphics::context> get_context_ref(void* address) {
            return *(ref<graphics::context>*)address;
        }
        void BasicVoxelEngine_Graphics_Context_DestroyRef(IntPtr address) {
            delete (ref<graphics::context>*)address;
        }
        void BasicVoxelEngine_Graphics_Context_MakeCurrent(IntPtr address) {
            auto context = get_context_ref(address);
            context->make_current();
        }
        void BasicVoxelEngine_Graphics_Context_DrawIndexed(IntPtr address, int32_t indexCount) {
            auto context = get_context_ref(address);
            context->draw_indexed((size_t)indexCount);
        }
    }
}