#pragma once
namespace bve {
    namespace managed {
        class wrapper : public ref_counted {
        public:
            wrapper(MonoDomain* domain);
            virtual ~wrapper() = default;
            MonoDomain* get_domain();
            virtual void* get() = 0;
            virtual MonoImage* get_image() = 0;
        private:
            MonoDomain* m_domain;
        };
        class object : public wrapper {
        public:
            static void handle_exception(ref<object> exception);
            object(MonoObject* _object, MonoDomain* domain);
            object(const std::string& string, MonoDomain* domain);
            virtual ~object() override;
            ref<object> get(MonoClassField* field);
            ref<object> get(MonoProperty* property);
            void set(MonoClassField* field, void* value);
            void set(MonoProperty* property, void* value);
            std::string get_string();
            template<typename T> T unbox() {
                MonoObject* _object = mono_gchandle_get_target(this->m_handle);
                void* pointer = mono_object_unbox(_object);
                return *(T*)pointer;
            }
            template<typename... Args> ref<object> invoke(MonoMethod* method, Args*... args) {
                if (!method) {
                    throw std::runtime_error("[managed object] attempted to call nullptr!");
                }
                MonoObject* _object = mono_gchandle_get_target(this->m_handle);
                std::vector<void*> args_vector = { std::forward<Args*>(args)... };
                MonoObject* exc = nullptr;
                MonoObject* returned = mono_runtime_invoke(method, _object, args_vector.data(), &exc);
                if (exc) {
                    ref<object> exception = ref<object>::create(exc, this->get_domain());
                    handle_exception(exception);
                }
                ref<object> returned_object;
                if (returned) {
                    returned_object = ref<object>::create(returned, this->get_domain());
                }
                return returned_object;
            }
            virtual void* get() override;
            virtual MonoImage* get_image() override;
        private:
            uint32_t m_handle;
        };
        class class_ : public wrapper {
        public:
            class_(MonoClass* _class, MonoDomain* domain);
            std::string get_name();
            void get_name(std::string& namespace_name, std::string& class_name);
            MonoMethod* get_method(const std::string& descriptor);
            MonoClassField* get_field(const std::string& name);
            MonoProperty* get_property(const std::string& name);
            template<typename... Args> ref<object> invoke(MonoMethod* method, Args*... args) {
                if (!method) {
                    throw std::runtime_error("[managed class] attempted to call nullptr!");
                }
                std::vector<void*> args_vector = { std::forward<Args*>(args)... };
                MonoObject* exc = nullptr;
                MonoObject* returned = mono_runtime_invoke(method, nullptr, args_vector.data(), &exc);
                if (exc) {
                    ref<object> exception = ref<object>::create(exc, this->get_domain());
                    object::handle_exception(exception);
                }
                ref<object> returned_object;
                if (returned) {
                    returned_object = ref<object>::create(returned, this->get_domain());
                }
                return returned_object;
            }
            virtual void* get() override;
            virtual MonoImage* get_image() override;
        private:
            MonoClass* m_class;
            MonoImage* m_image;
        };
        class assembly;
        class type : public wrapper {
        public:
            type(MonoType* type, MonoDomain* domain);
            ref<class_> get_class();
            virtual void* get() override;
            virtual MonoImage* get_image() override;
        private:
            MonoType* m_type;
        };
        class assembly : public wrapper {
        public:
            static ref<assembly> get_assembly(ref<wrapper> wrapper_object);
            assembly(MonoAssembly* _assembly, MonoDomain* domain);
            ref<class_> get_class(const std::string& name);
            ref<class_> get_class(const std::string& namespace_name, const std::string& class_name);
            MonoMethod* get_method(const std::string& descriptor);
            virtual void* get() override;
            virtual MonoImage* get_image() override;
        private:
            MonoAssembly* m_assembly;
            MonoImage* m_image;
        };
    }
}