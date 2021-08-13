#pragma once
namespace bve {
    struct namespaced_name {
    public:
        struct hash_function {
            size_t operator()(const namespaced_name& key) const {
                std::hash<std::string> hasher;
                return hasher(key.get_full_name());
            }
        };
        namespaced_name() = default;
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
            return this->namespace_name + ":" + this->local_name;
        }
        std::string namespace_name, local_name;
    private:
        void convert(const std::string& full_name) {
            size_t pos = full_name.find(':');
            if (pos != std::string::npos) {
                this->namespace_name = full_name.substr(0, pos);
                this->local_name = full_name.substr(pos + 1);
            } else {
                this->local_name = full_name;
            }
        }
    };
    template<typename T> class object_register : public ref_counted {
    public:
        using stored_type = T;
        using element_type = ref<stored_type>;
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
            auto index = this->get_index(name);
            if (!index) {
                throw std::runtime_error("[register] could not find specified register entry!");
            }
            return (*this)[*index];
        }
        const element_type& operator[](size_type index) const {
            auto it = this->m_objects.begin();
            std::advance(it, index);
            return *it;
        }
        std::optional<namespaced_name> get_name(size_type index) {
            for (const auto& pair : this->m_name_map) {
                if (pair.second == index) {
                    return pair.first;
                }
            }
            return std::optional<namespaced_name>();
        }
        std::optional<size_type> get_index(const namespaced_name& name) {
            if (this->m_name_map.find(name) == this->m_name_map.end()) {
                return std::optional<size_type>();
            }
            return this->m_name_map[name];
        }
        std::vector<namespaced_name> get_names() {
            std::vector<namespaced_name> names;
            for (const auto& pair : this->m_name_map) {
                names.push_back(pair.first);
            }
            return names;
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
        object_register() { }
        std::list<element_type> m_objects;
        std::unordered_map<namespaced_name, size_type, namespaced_name::hash_function> m_name_map;
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
                auto register_ = ref<ref_counted>(new object_register<T>);
                this->m_registers.insert({ hash_code, register_ });
                return (object_register<T>&)*register_;
            } else {
                return (object_register<T>&)*this->m_registers[hash_code];
            }
        }
    private:
        registry() { }
        std::unordered_map<size_t, ref<ref_counted>> m_registers;
    };
}