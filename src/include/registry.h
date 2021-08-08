#pragma once
namespace bve {
    struct namespaced_name {
    public:
        namespaced_name(const char* full_name) {
            this->convert(full_name);
        }
        namespaced_name(const std::string& full_name) {
            this->convert(full_name);
        }
        namespaced_name(const std::string& namespace_name, const std::string& local_name) {
            this->namespace_name = namespace_name;
            this->local_name = local_name;
        }
        namespaced_name& operator=(const char* full_name) {
            this->convert(full_name);
            return *this;
        }
        namespaced_name& operator=(const std::string& full_name) {
            this->convert(full_name);
            return *this;
        }
        bool operator==(const namespaced_name& other) const {
            return this->namespace_name == other.namespace_name && this->local_name == other.local_name;
        }
        bool operator!=(const namespaced_name& other) const {
            return !(*this == other);
        }
        std::string get_full_name() const {
            return this->namespace_name + this->local_name;
        }
        std::string namespace_name, local_name;
    private:
        void convert(const std::string& full_name) {
            size_t pos = full_name.find(':');
            if (pos != std::string::npos) {
                this->namespace_name = full_name.substr(0, pos);
                this->local_name = full_name.substr(pos);
            } else {
                this->local_name = full_name;
            }
        }
    };
    class __object_register_base__ {
    public:
        virtual ~__object_register_base__() { }
    };
    template<typename T> class object_register : public __object_register_base__ {
    public:
        using stored_type = T;
        using element_type = std::shared_ptr<stored_type>;
        using size_type = typename std::list<element_type>::size_type;
        object_register(const object_register<T>&) = delete;
        object_register<T>& operator=(const object_register<T>&) = delete;
        void add(element_type object, const namespaced_name& name) {
            size_type index = *this += object;
            this->set_name(name, index);
        }
        void add(element_type object, namespaced_name&& name) {
            this->add(object, name);
        }
        size_type operator+=(element_type object) {
            size_type index = this->m_objects.size();
            this->m_objects.push_back(object);
            return index;
        }
        void operator-=(element_type object) {
            this->m_objects.remove(object);
        }
        void operator-=(const namespaced_name& name) {
            if (this->m_name_map.find(name) == this->m_name_map.end()) {
                return;
            }
            size_t index = this->m_name_map[name];
            auto& element = (*this)[index];
            this->m_objects.remove(element);
        }
        void set_name(const namespaced_name& name, size_type index) {
            this->m_name_map[name] = index;
        }
        size_type size() const {
            return this->m_objects.size();
        }
        element_type& operator[](size_type index) {
            auto it = this->m_objects.begin();
            std::advance(it, index);
            return *it;
        }
        element_type& operator[](const namespaced_name& name) {
            if (this->m_name_map.find(name) == this->m_name_map.end()) {
                throw std::runtime_error("Could not find specified register entry!");
            }
            size_type index = this->m_name_map[name];
            return (*this)[index];
        }
        const element_type& operator[](size_type index) const {
            auto it = this->m_objects.begin();
            std::advance(it, index);
            return *it;
        }
        typename std::list<element_type>::iterator begin() {
            return this->m_objects.begin();
        }
        typename std::list<element_type>::iterator end() {
            return this->m_objects.end();
        }
        typename std::list<element_type>::const_iterator begin() const {
            return this->m_objects.begin();
        }
        typename std::list<element_type>::const_iterator end() const {
            return this->m_objects.end();
        }
    private:
        struct hash_function {
            size_t operator()(const namespaced_name& key) const {
                std::hash<std::string> hasher;
                return hasher(key.get_full_name());
            }
        };
        object_register() { }
        std::list<element_type> m_objects;
        std::unordered_map<namespaced_name, size_type, hash_function> m_name_map;
        friend class registry;
    };
    class registry {
    public:
        static registry& get() {
            static registry instance;
            return instance;
        }
        registry(const registry&) = delete;
        registry& operator=(const registry&) = delete;
        template<typename T> object_register<T>& get_register() {
            size_t hash_code = typeid(T).hash_code();
            if (this->m_registers.find(hash_code) == this->m_registers.end()) {
                auto register_ = std::shared_ptr<__object_register_base__>(new object_register<T>);
                this->m_registers.insert({ hash_code, register_ });
                return (object_register<T>&)*register_;
            } else {
                return (object_register<T>&)*this->m_registers[hash_code];
            }
        }
    private:
        registry() { }
        std::unordered_map<size_t, std::shared_ptr<__object_register_base__>> m_registers;
    };
}