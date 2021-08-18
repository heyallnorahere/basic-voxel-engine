#include "bve_pch.h"
#include "code_host.h"
namespace bve {
    namespace managed {
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
        wrapper::wrapper(MonoDomain* domain) {
            this->m_domain = domain;
        }
        MonoDomain* wrapper::get_domain() {
            return this->m_domain;
        }
        void object::check_exception(ref<object> exception) {
            ref<class_> exception_class = ref<class_>::create(mono_get_exception_class(), exception->get_domain());
            // todo: read data
        }
        object::object(MonoObject* _object, MonoDomain* domain) : wrapper(domain) {
            this->m_handle = mono_gchandle_new(_object, false);
        }
        object::~object() {
            mono_gchandle_free(this->m_handle);
        }
        void* object::get() {
            return mono_gchandle_get_target(this->m_handle);
        }
        MonoImage* object::get_image() {
            MonoObject* _object = mono_gchandle_get_target(this->m_handle);
            MonoClass* _class = mono_object_get_class(_object);
            return mono_class_get_image(_class);
        }
        class_::class_(MonoClass* _class, MonoDomain* domain) : wrapper(domain) {
            this->m_class = _class;
        }
        void* class_::get() {
            return this->m_class;
        }
        MonoImage* class_::get_image() {
            return mono_class_get_image(this->m_class);
        }
        type::type(MonoType* type, MonoDomain* domain) : wrapper(domain) {
            this->m_type = type;
        }
        ref<class_> type::get_class() {
            MonoClass* _class = mono_type_get_class(this->m_type);
            return ref<class_>::create(_class, this->get_domain());
        }
        void* type::get() {
            return this->m_type;
        }
        MonoImage* type::get_image() {
            MonoClass* _class = mono_type_get_class(this->m_type);
            return mono_class_get_image(_class);
        }
        assembly::assembly(MonoAssembly* _assembly, MonoDomain* domain) : wrapper(domain) {
            this->m_assembly = _assembly;
            this->m_image = mono_assembly_get_image(this->m_assembly);
        }
        ref<class_> assembly::get_class(const std::string& name) {
            std::string namespace_name, class_name;
            parse_name(name, namespace_name, class_name);
            MonoClass* _class = mono_class_from_name(this->m_image, namespace_name.c_str(), class_name.c_str());
            return ref<class_>::create(_class, this->get_domain());
        }
        void* assembly::get() {
            return this->m_assembly;
        }
        MonoImage* assembly::get_image() {
            return this->m_image;
        }
    }
}