#include "bve_pch.h"
#include "code_host.h"
#include "script_wrappers.h"
#include "util.h"
namespace bve {
    static void parse_name(const std::string& full_name, std::string& namespace_name, std::string& class_name) {
        size_t period_position = full_name.find_last_of('.');
        if (period_position == std::string::npos) {
            namespace_name.clear();
            class_name = full_name;
        } else {
            namespace_name = full_name.substr(0, period_position);
            class_name = full_name.substr(period_position + 1);
        }
    }
    namespace managed {
        wrapper::wrapper(MonoDomain* domain) {
            this->m_domain = domain;
        }
        MonoDomain* wrapper::get_domain() {
            return this->m_domain;
        }
        void object::handle_exception(ref<object> exception) {
            ref<class_> object_class = class_::get_class(exception);
            ref<class_> exception_class = ref<class_>::create(mono_get_exception_class(), exception->get_domain());
            auto property = exception_class->get_property("Message");
            std::string message = exception->get(property)->get_string();
            property = exception_class->get_property("Source");
            ref<object> source_object = exception->get(property);
            property = exception_class->get_property("StackTrace");
            ref<object> stacktrace_object = exception->get(property);
            spdlog::error("[managed object] threw {0}: {1}", object_class->get_name(), message);
            if (source_object) {
                std::string source = source_object->get_string();
                spdlog::error("[managed object] from assembly: {0}", source);
            }
            if (stacktrace_object) {
                std::string stacktrace = stacktrace_object->get_string();
                std::stringstream ss(stacktrace);
                std::string line;
                while (std::getline(ss, line)) {
                    spdlog::error("[managed object] {0}", line);
                }
            }
            property = exception_class->get_property("InnerException");
            ref<object> inner_exception = exception->get(property);
            if (inner_exception) {
                spdlog::info("[managed object] inner exception:");
                handle_exception(inner_exception);
            }
        }
        object::object(MonoObject* _object, MonoDomain* domain) : wrapper(domain) {
            this->m_handle = mono_gchandle_new(_object, false);
        }
        object::object(const std::string& string, MonoDomain* domain) : wrapper(domain) {
            MonoString* str = mono_string_new(domain, string.c_str());
            this->m_handle = mono_gchandle_new((MonoObject*)str, false);
        }
        object::~object() {
            mono_gchandle_free(this->m_handle);
        }
        ref<object> object::get(MonoClassField* field) {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            MonoObject* value = mono_field_get_value_object(this->get_domain(), field, _object);
            ref<object> returned_object;
            if (value) {
                returned_object = ref<object>::create(value, this->get_domain());
            }
            return returned_object;
        }
        void object::set(MonoClassField* field, void* value) {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            mono_field_set_value(_object, field, value);
        }
        std::string object::get_string() {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            MonoString* string = (MonoString*)_object;
            return std::string(mono_string_to_utf8(string));
        }
        void* object::get() {
            return mono_gchandle_get_target(this->m_handle);
        }
        MonoImage* object::get_image() {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            MonoClass* _class = mono_object_get_class(_object);
            return mono_class_get_image(_class);
        }
        delegate::delegate(ref<object> _object) : delegate(_object->get(), _object->get_domain()) { }
        delegate::delegate(void* _delegate, MonoDomain* domain) : wrapper(domain) {
            this->m_handle = mono_gchandle_new((MonoObject*)_delegate, false);
        }
        delegate::~delegate() {
            mono_gchandle_free(this->m_handle);
        }
        void* delegate::get() {
            return mono_gchandle_get_target(this->m_handle);
        }
        MonoImage* delegate::get_image() {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            MonoClass* _class = mono_object_get_class(_object);
            return mono_class_get_image(_class);
        }
        ref<class_> class_::get_class(ref<object> _object) {
            auto object_ptr = (MonoObject*)_object->get();
            MonoClass* class_ptr = mono_object_get_class(object_ptr);
            return ref<class_>::create(class_ptr, _object->get_domain());
        }
        class_::class_(MonoClass* _class, MonoDomain* domain) : wrapper(domain) {
            this->m_class = _class;
        }
        std::string class_::get_name() {
            std::string namespace_name, class_name;
            this->get_name(namespace_name, class_name);
            return namespace_name + "." + class_name;
        }
        void class_::get_name(std::string& namespace_name, std::string& class_name) {
            namespace_name = mono_class_get_namespace(this->m_class);
            class_name = mono_class_get_name(this->m_class);
        }
        MonoMethod* class_::get_method(const std::string& descriptor) {
            MonoMethodDesc* desc = mono_method_desc_new(descriptor.c_str(), false);
            return mono_method_desc_search_in_class(desc, this->m_class);
        }
        MonoClassField* class_::get_field(const std::string& name) {
            return mono_class_get_field_from_name(this->m_class, name.c_str());
        }
        MonoProperty* class_::get_property(const std::string& name) {
            return mono_class_get_property_from_name(this->m_class, name.c_str());
        }
        bool class_::derives_from(ref<class_> cls) {
            ref<code_host> host = code_host::current();
            ref<class_> helpers = host->find_class("BasicVoxelEngine.Helpers");
            MonoMethod* method = helpers->get_method("*:DerivesFrom");
            auto reftype = type::get_type(this)->get_object();
            auto base_reftype = type::get_type(cls)->get_object();
            auto result = managed::class_::invoke(method, reftype, base_reftype);
            return result->unbox<bool>();
        }
        bool class_::is_value_type() {
            return mono_class_is_valuetype(this->m_class);
        }
        size_t class_::data_size() {
            ref<code_host> host = code_host::current();
            ref<class_> helpers = host->find_class("BasicVoxelEngine.Helpers");
            MonoMethod* method = helpers->get_method("*:SizeOf");
            auto reftype = type::get_type(this)->get_object();
            auto result = managed::class_::invoke(method, reftype);
            return (size_t)result->unbox<int32_t>();
        }
        void* class_::get() {
            return this->m_class;
        }
        MonoImage* class_::get_image() {
            return mono_class_get_image(this->m_class);
        }
        ref<type> type::get_type(ref<class_> _class) {
            MonoClass* _class_ = (MonoClass*)_class->get();
            MonoType* _type = mono_class_get_type(_class_);
            return ref<type>::create(_type, _class->get_domain());
        }
        type::type(MonoType* _type, MonoDomain* domain) : wrapper(domain) {
            this->m_type = _type;
        }
        type::type(MonoReflectionType* _type, MonoDomain* domain) : wrapper(domain) {
            this->m_type = mono_reflection_type_get_type(_type);
        }
        ref<class_> type::get_class() {
            MonoClass* _class = mono_type_get_class(this->m_type);
            return ref<class_>::create(_class, this->get_domain());
        }
        MonoReflectionType* type::get_object() {
            return mono_type_get_object(this->get_domain(), this->m_type);
        }
        void* type::get() {
            return this->m_type;
        }
        MonoImage* type::get_image() {
            MonoClass* _class = mono_type_get_class(this->m_type);
            return mono_class_get_image(_class);
        }
        ref<assembly> assembly::get_assembly(ref<wrapper> wrapper_object) {
            MonoImage* image = wrapper_object->get_image();
            MonoAssembly* _assembly = mono_image_get_assembly(image);
            MonoDomain* domain = wrapper_object->get_domain();
            return ref<assembly>::create(_assembly, domain);
        }
        assembly::assembly(MonoAssembly* _assembly, MonoDomain* domain) : wrapper(domain) {
            this->m_assembly = _assembly;
            this->m_image = mono_assembly_get_image(this->m_assembly);
        }
        ref<class_> assembly::get_class(const std::string& name) {
            std::string namespace_name, class_name;
            parse_name(name, namespace_name, class_name);
            return this->get_class(namespace_name, class_name);
        }
        ref<class_> assembly::get_class(const std::string& namespace_name, const std::string& class_name) {
            MonoClass* _class = mono_class_from_name(this->m_image, namespace_name.c_str(), class_name.c_str());
            return ref<class_>::create(_class, this->get_domain());
        }
        MonoMethod* assembly::get_method(const std::string& descriptor) {
            MonoMethodDesc* desc = mono_method_desc_new(descriptor.c_str(), false);
            return mono_method_desc_search_in_image(desc, this->m_image);
        }
        MonoReflectionAssembly* assembly::get_object() {
            return mono_assembly_get_object(this->get_domain(), this->m_assembly);
        }
        void* assembly::get() {
            return this->m_assembly;
        }
        MonoImage* assembly::get_image() {
            return this->m_image;
        }
    }
    template<typename T> static std::pair<std::string, void*> pair(const std::string& name, T function) {
        return { name, (void*)function };
    }
    std::unordered_map<std::string, void*> code_host::get_script_wrappers() {
        using namespace script_wrappers;
        return {
            pair("BasicVoxelEngine.Application::Quit", BasicVoxelEngine_Application_Quit),
            pair("BasicVoxelEngine.Application::GetDeltaTime", BasicVoxelEngine_Application_GetDeltaTime),
            pair("BasicVoxelEngine.Application::GetWorld_Native", BasicVoxelEngine_Application_GetWorld),
            pair("BasicVoxelEngine.Application::GetInputManager_Native", BasicVoxelEngine_Application_GetInputManager),
            pair("BasicVoxelEngine.Application::GetFactory_Native", BasicVoxelEngine_Application_GetFactory),
            pair("BasicVoxelEngine.Application::GetWindow_Native", BasicVoxelEngine_Application_GetWindow),

            pair("BasicVoxelEngine.Logger::PrintDebug_Native", BasicVoxelEngine_Logger_PrintDebug),
            pair("BasicVoxelEngine.Logger::PrintInfo_Native", BasicVoxelEngine_Logger_PrintInfo),
            pair("BasicVoxelEngine.Logger::PrintWarning_Native", BasicVoxelEngine_Logger_PrintWarning),
            pair("BasicVoxelEngine.Logger::PrintError_Native", BasicVoxelEngine_Logger_PrintError),

            pair("BasicVoxelEngine.Graphics.Factory::DestroyRef_Native", BasicVoxelEngine_Graphics_Factory_DestroyRef),
            pair("BasicVoxelEngine.Graphics.Factory::CreateTexture_Native", BasicVoxelEngine_Graphics_Factory_CreateTexture),
            pair("BasicVoxelEngine.Graphics.Factory::CreateContext_Native", BasicVoxelEngine_Graphics_Factory_CreateContext),
            pair("BasicVoxelEngine.Graphics.Factory::CreateShader_Native", BasicVoxelEngine_Graphics_Factory_CreateShader),

            pair("BasicVoxelEngine.Model::LoadModel_Native", BasicVoxelEngine_Model_LoadModel),
            pair("BasicVoxelEngine.Model::DestroyRef_Native", BasicVoxelEngine_Model_DestroyRef),
            pair("BasicVoxelEngine.Model::GetMeshCount_Native", BasicVoxelEngine_Model_GetMeshCount),
            pair("BasicVoxelEngine.Model::GetMesh_Native", BasicVoxelEngine_Model_GetMesh),
            pair("BasicVoxelEngine.Model::GetTexture_Native", BasicVoxelEngine_Model_GetTexture),
            pair("BasicVoxelEngine.Model::GetVertexCount_Native", BasicVoxelEngine_Model_GetVertexCount),
            pair("BasicVoxelEngine.Model::GetVertex_Native", BasicVoxelEngine_Model_GetVertex),
            pair("BasicVoxelEngine.Model::GetIndexCount_Native", BasicVoxelEngine_Model_GetIndexCount),
            pair("BasicVoxelEngine.Model::GetIndex_Native", BasicVoxelEngine_Model_GetIndex),

            pair("BasicVoxelEngine.MeshTextureOffsetData::Destroy_Native", BasicVoxelEngine_MeshTextureOffsetData_Destroy),
            pair("BasicVoxelEngine.MeshTextureOffsetData::GetCount_Native", BasicVoxelEngine_MeshTextureOffsetData_GetCount),
            pair("BasicVoxelEngine.MeshTextureOffsetData::GetKey_Native", BasicVoxelEngine_MeshTextureOffsetData_GetKey),
            pair("BasicVoxelEngine.MeshTextureOffsetData::GetValue_Native", BasicVoxelEngine_MeshTextureOffsetData_GetValue),

            pair("BasicVoxelEngine.AssetManager::GetAssetPath", BasicVoxelEngine_AssetManager_GetAssetPath),

            pair("BasicVoxelEngine.Lighting.Light::Destroy_Native", BasicVoxelEngine_Lighting_Light_Destroy),
            pair("BasicVoxelEngine.Lighting.Light::SetColor_Native", BasicVoxelEngine_Lighting_Light_SetColor),
            pair("BasicVoxelEngine.Lighting.Light::SetAmbientStrength_Native", BasicVoxelEngine_Lighting_Light_SetAmbientStrength),
            pair("BasicVoxelEngine.Lighting.Light::SetSpecularStrength_Native", BasicVoxelEngine_Lighting_Light_SetSpecularStrength),
            pair("BasicVoxelEngine.Lighting.Light::GetType_Native", BasicVoxelEngine_Lighting_Light_GetType),

            pair("BasicVoxelEngine.Lighting.Spotlight::Create_Native", BasicVoxelEngine_Lighting_Spotlight_Create),
            pair("BasicVoxelEngine.Lighting.Spotlight::SetDirection_Native", BasicVoxelEngine_Lighting_Spotlight_SetDirection),
            pair("BasicVoxelEngine.Lighting.Spotlight::SetCutoff_Native", BasicVoxelEngine_Lighting_Spotlight_SetCutoff),

            pair("BasicVoxelEngine.Lighting.PointLight::Create_Native", BasicVoxelEngine_Lighting_PointLight_Create),
            pair("BasicVoxelEngine.Lighting.PointLight::SetConstant_Native", BasicVoxelEngine_Lighting_PointLight_SetConstant),
            pair("BasicVoxelEngine.Lighting.PointLight::SetLinear_Native", BasicVoxelEngine_Lighting_PointLight_SetLinear),
            pair("BasicVoxelEngine.Lighting.PointLight::SetQuadratic_Native", BasicVoxelEngine_Lighting_PointLight_SetQuadratic),

            pair("BasicVoxelEngine.World::Destroy_Native", BasicVoxelEngine_World_Destroy),
            pair("BasicVoxelEngine.World::GetBlock_Native", BasicVoxelEngine_World_GetBlock),
            pair("BasicVoxelEngine.World::SetBlock_Native", BasicVoxelEngine_World_SetBlock),
            pair("BasicVoxelEngine.World::AddOnBlockChangedCallback_Native", BasicVoxelEngine_World_AddOnBlockChangedCallback),
            pair("BasicVoxelEngine.World::CreateEntity_Native", BasicVoxelEngine_World_CreateEntity),

            pair("BasicVoxelEngine.Entity::AddComponent_Native", BasicVoxelEngine_Entity_AddComponent),
            pair("BasicVoxelEngine.Entity::GetComponent_Native", BasicVoxelEngine_Entity_GetComponent),
            pair("BasicVoxelEngine.Entity::HasComponent_Native", BasicVoxelEngine_Entity_HasComponent),
            pair("BasicVoxelEngine.Entity::RemoveComponent_Native", BasicVoxelEngine_Entity_RemoveComponent),
            pair("BasicVoxelEngine.Entity::RegisterComponents_Native", BasicVoxelEngine_Entity_RegisterComponents),

            pair("BasicVoxelEngine.Components.TransformComponent::GetTranslation_Native", BasicVoxelEngine_Components_TransformComponent_GetTranslation),
            pair("BasicVoxelEngine.Components.TransformComponent::SetTranslation_Native", BasicVoxelEngine_Components_TransformComponent_SetTranslation),
            pair("BasicVoxelEngine.Components.TransformComponent::GetRotation_Native", BasicVoxelEngine_Components_TransformComponent_GetRotation),
            pair("BasicVoxelEngine.Components.TransformComponent::SetRotation_Native", BasicVoxelEngine_Components_TransformComponent_SetRotation),
            pair("BasicVoxelEngine.Components.TransformComponent::GetScale_Native", BasicVoxelEngine_Components_TransformComponent_GetScale),
            pair("BasicVoxelEngine.Components.TransformComponent::SetScale_Native", BasicVoxelEngine_Components_TransformComponent_SetScale),

            pair("BasicVoxelEngine.Components.CameraComponent::GetDirection_Native", BasicVoxelEngine_Components_CameraComponent_GetDirection),
            pair("BasicVoxelEngine.Components.CameraComponent::SetDirection_Native", BasicVoxelEngine_Components_CameraComponent_SetDirection),
            pair("BasicVoxelEngine.Components.CameraComponent::GetUp_Native", BasicVoxelEngine_Components_CameraComponent_GetUp),
            pair("BasicVoxelEngine.Components.CameraComponent::SetUp_Native", BasicVoxelEngine_Components_CameraComponent_SetUp),
            pair("BasicVoxelEngine.Components.CameraComponent::GetPrimary_Native", BasicVoxelEngine_Components_CameraComponent_GetPrimary),
            pair("BasicVoxelEngine.Components.CameraComponent::SetPrimary_Native", BasicVoxelEngine_Components_CameraComponent_SetPrimary),
            pair("BasicVoxelEngine.Components.CameraComponent::GetNearPlane_Native", BasicVoxelEngine_Components_CameraComponent_GetNearPlane),
            pair("BasicVoxelEngine.Components.CameraComponent::SetNearPlane_Native", BasicVoxelEngine_Components_CameraComponent_SetNearPlane),
            pair("BasicVoxelEngine.Components.CameraComponent::GetFarPlane_Native", BasicVoxelEngine_Components_CameraComponent_GetFarPlane),
            pair("BasicVoxelEngine.Components.CameraComponent::SetFarPlane_Native", BasicVoxelEngine_Components_CameraComponent_SetFarPlane),

            pair("BasicVoxelEngine.Components.ScriptComponent::Bind_Native", BasicVoxelEngine_Components_ScriptComponent_Bind),
            pair("BasicVoxelEngine.Components.ScriptComponent::GetScriptCount_Native", BasicVoxelEngine_Components_ScriptComponent_GetScriptCount),
            pair("BasicVoxelEngine.Components.ScriptComponent::GetScript_Native", BasicVoxelEngine_Components_ScriptComponent_GetScript),

            pair("BasicVoxelEngine.InputManager::DestroyRef_Native", BasicVoxelEngine_InputManager_DestroyRef),
            pair("BasicVoxelEngine.InputManager::GetMouseEnabled_Native", BasicVoxelEngine_InputManager_GetMouseEnabled),
            pair("BasicVoxelEngine.InputManager::SetMouseEnabled_Native", BasicVoxelEngine_InputManager_SetMouseEnabled),
            pair("BasicVoxelEngine.InputManager::GetMouse_Native", BasicVoxelEngine_InputManager_GetMouse),
            pair("BasicVoxelEngine.InputManager::GetKey_Native", BasicVoxelEngine_InputManager_GetKey),
            pair("BasicVoxelEngine.InputManager::GetMouseButton_Native", BasicVoxelEngine_InputManager_GetMouseButton),

            pair("BasicVoxelEngine.Graphics.ImageData::Load_Native", BasicVoxelEngine_Graphics_ImageData_Load),
            pair("BasicVoxelEngine.Graphics.ImageData::Destroy_Native", BasicVoxelEngine_Graphics_ImageData_Destroy),
            pair("BasicVoxelEngine.Graphics.ImageData::GetWidth_Native", BasicVoxelEngine_Graphics_ImageData_GetWidth),
            pair("BasicVoxelEngine.Graphics.ImageData::GetHeight_Native", BasicVoxelEngine_Graphics_ImageData_GetHeight),
            pair("BasicVoxelEngine.Graphics.ImageData::GetChannels_Native", BasicVoxelEngine_Graphics_ImageData_GetChannels),
            pair("BasicVoxelEngine.Graphics.ImageData::GetByte_Native", BasicVoxelEngine_Graphics_ImageData_GetByte),

            pair("BasicVoxelEngine.Graphics.Texture::DestroyRef_Native", BasicVoxelEngine_Graphics_Texture_DestroyRef),
            pair("BasicVoxelEngine.Graphics.Texture::GetSize_Native", BasicVoxelEngine_Graphics_Texture_GetSize),
            pair("BasicVoxelEngine.Graphics.Texture::GetChannels_Native", BasicVoxelEngine_Graphics_Texture_GetChannels),

            pair("BasicVoxelEngine.Window::DestroyRef_Native", BasicVoxelEngine_Window_DestroyRef),
            pair("BasicVoxelEngine.Window::GetContext_Native", BasicVoxelEngine_Window_GetContext),
            pair("BasicVoxelEngine.Window::GetFramebufferSize_Native", BasicVoxelEngine_Window_GetFramebufferSize),

            pair("BasicVoxelEngine.Graphics.Shader::DestroyRef_Native", BasicVoxelEngine_Graphics_Shader_DestroyRef),
            pair("BasicVoxelEngine.Graphics.Shader::Reload_Native", BasicVoxelEngine_Graphics_Shader_Reload),
            pair("BasicVoxelEngine.Graphics.Shader::Bind_Native", BasicVoxelEngine_Graphics_Shader_Bind),
            pair("BasicVoxelEngine.Graphics.Shader::Unbind_Native", BasicVoxelEngine_Graphics_Shader_Unbind),
            pair("BasicVoxelEngine.Graphics.Shader::Set_Native", BasicVoxelEngine_Graphics_Shader_Set),
            pair("BasicVoxelEngine.Graphics.Shader::Get_Native", BasicVoxelEngine_Graphics_Shader_Get),
            pair("BasicVoxelEngine.Graphics.Shader::InitializeUniforms_Native", BasicVoxelEngine_Graphics_Shader_InitializeUniforms),

            pair("BasicVoxelEngine.Graphics.Context::DestroyRef_Native", BasicVoxelEngine_Graphics_Context_DestroyRef),
            pair("BasicVoxelEngine.Graphics.Context::MakeCurrent_Native", BasicVoxelEngine_Graphics_Context_MakeCurrent),
            pair("BasicVoxelEngine.Graphics.Context::DrawIndexed_Native", BasicVoxelEngine_Graphics_Context_DrawIndexed),

            pair("BasicVoxelEngine.Mesh::AllocVertexBuffer_Native", BasicVoxelEngine_Mesh_AllocVertexBuffer),
            pair("BasicVoxelEngine.Mesh::CopyVertex_Native", BasicVoxelEngine_Mesh_CopyVertex),
            pair("BasicVoxelEngine.Mesh::FreeVertexBuffer_Native", BasicVoxelEngine_Mesh_FreeVertexBuffer),

            pair("BasicVoxelEngine.CommandList::Create_Native", BasicVoxelEngine_CommandList_Create),
            pair("BasicVoxelEngine.CommandList::Destroy_Native", BasicVoxelEngine_CommandList_Destroy),
            pair("BasicVoxelEngine.CommandList::AddMesh_Native", BasicVoxelEngine_CommandList_AddMesh),
            pair("BasicVoxelEngine.CommandList::Close_Native", BasicVoxelEngine_CommandList_Close),

            pair("BasicVoxelEngine.Renderer::DestroyRef_Native", BasicVoxelEngine_Renderer_DestroyRef),
            pair("BasicVoxelEngine.Renderer::Render_Native", BasicVoxelEngine_Renderer_Render),
            pair("BasicVoxelEngine.Renderer::SetShader_Native", BasicVoxelEngine_Renderer_SetShader),
            pair("BasicVoxelEngine.Renderer::SetTexture_Native", BasicVoxelEngine_Renderer_SetTexture),

            pair("BasicVoxelEngine.Helpers::AreRefsEqual", BasicVoxelEngine_Helpers_AreRefsEqual),
        };
    }
    static ref<code_host> current_code_host;
    void code_host::remove_current() {
        current_code_host.reset();
    }
    ref<code_host> code_host::current() {
        return current_code_host;
    }
    code_host::code_host() {
        mono_config_parse(nullptr);
        mono_set_assemblies_path(MONO_ASSEMBLIES);
        this->m_domain = mono_jit_init(BVE_TARGET_NAME);
        this->make_current();
        MonoAssembly* corlib = mono_image_get_assembly(mono_get_corlib());
        this->m_assemblies.push_back(ref<managed::assembly>::create(corlib, this->m_domain));
    }
    code_host::~code_host() {
        mono_jit_cleanup(this->m_domain);
    }
    void code_host::make_current() {
        current_code_host = ref<code_host>(this);
        if (!mono_domain_set(this->m_domain, false)) {
            throw std::runtime_error("[code host] could not set the domain");
        }
    }
    MonoDomain* code_host::get_domain() {
        return this->m_domain;
    }
    void code_host::load_assembly(const fs::path& path, bool ref_only) {
        std::string string_path = path.string();
        // old-fashioned c-style file reading
        FILE* f = fopen(string_path.c_str(), "rb");
        if (!f) {
            throw std::runtime_error("[code host] could not open assembly");
        }
        fseek(f, 0, SEEK_END);
        size_t file_size = (size_t)ftell(f);
        rewind(f);
        char* file_data = (char*)malloc(file_size * sizeof(char));
        if (!file_data) {
            fclose(f);
            throw std::runtime_error("[code host] application ran out of memory");
        }
        util::zero(file_data, file_size * sizeof(char));
        size_t bytes_read = fread(file_data, sizeof(char), file_size, f);
        if (bytes_read != file_size) {
            fclose(f);
            free(file_data);
            throw std::runtime_error("[code host] could not read entire assembly");
        }
        fclose(f);
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(file_data, file_size * sizeof(char), true, &status, ref_only);
        free(file_data);
        if (status != MONO_IMAGE_OK) {
            return;
        }
        MonoAssembly* assembly = mono_assembly_load_from_full(image, string_path.c_str(), &status, false);
        mono_image_close(image);
        this->m_assemblies.push_back(ref<managed::assembly>::create(assembly, this->m_domain));
    }
    std::vector<ref<managed::assembly>> code_host::get_loaded_assemblies() {
        return this->m_assemblies;
    }
    ref<managed::class_> code_host::find_class(const std::string& name) {
        std::string namespace_name, class_name;
        parse_name(name, namespace_name, class_name);
        return this->find_class(namespace_name, class_name);
    }
    ref<managed::class_> code_host::find_class(const std::string& namespace_name, const std::string& class_name) {
        for (auto assembly : this->m_assemblies) {
            ref<managed::class_> current_class = assembly->get_class(namespace_name, class_name);
            if (current_class && current_class->get()) {
                return current_class;
            }
        }
        return nullptr;
    }
}