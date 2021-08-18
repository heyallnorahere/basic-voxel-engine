#pragma once
namespace bve {
    class code_host;
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
            static void check_exception(ref<object> exception);
            object(MonoObject* _object, MonoDomain* domain);
            virtual ~object() override;
            template<typename... Args> ref<object> invoke(MonoMethod* method, Args*... args) {
                MonoObject* _object = mono_gchandle_get_target(this->m_handle);
                std::vector<void*> args_vector = { std::forward<Args*>(args)... };
                MonoObject* exc = nullptr;
                MonoObject* returned = mono_runtime_invoke(method, _object, args_vector.data(), &exc);
                if (exc) {
                    ref<object> exception = ref<object>::create(exc, this->get_domain());
                    check_exception(exception);
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
            assembly(MonoAssembly* _assembly, MonoDomain* domain);
            ref<class_> get_class(const std::string& name);
            virtual void* get() override;
            virtual MonoImage* get_image() = 0;
        private:
            MonoAssembly* m_assembly;
            MonoImage* m_image;
            friend class ::bve::code_host;
        };
    }
}