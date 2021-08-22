#pragma once
namespace bve {
    // A namespaced_name is an identifier in the form of "namespace:name".
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

    // An object register creates an ID from a namespaced_name
    // and stores an object for that ID.
    //
    // Users can:
    //  * Insert a new object with or without a name, the ID is returned
    //  * Get the object from the ID
    //  * Iterate over all IDs
    //  * Get a list of all of the names
    template<typename T> class object_register : public ref_counted {
    public:
        using stored_type = T;
        using element_type = ref<stored_type>;
        using id_type = typename std::vector<element_type>::size_type;

        object_register(const object_register<T>&) = delete;
        object_register<T>& operator=(const object_register<T>&) = delete;

        void add(element_type object, const namespaced_name& name) {
            id_type id = this->m_objects.size();
            this->m_objects.push_back(object);
            this->m_name_map[name] = id;
            this->m_id_map[id] = name;
        }
        id_type size() const {
            return this->m_objects.size();
        }
        element_type& operator[](id_type id) {
            return this->m_objects.at(id);
        }
        element_type& operator[](const namespaced_name& name) {
            auto id = this->m_name_map.at(name);
            return (*this)[id];
        }
        std::optional<namespaced_name> get_name(id_type id) {
            auto it = this->m_id_map.find(id);
            if (it == this->m_id_map.end()) {
                return std::nullopt;
            }
            return it->second;
        }
        std::optional<id_type> get_index(const namespaced_name& name) {
            auto it = this->m_name_map.find(name);
            if (it == this->m_name_map.end()) {
                return std::nullopt;
            }
            return it->second;
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
        friend class registry;
        object_register() {}

        std::vector<element_type> m_objects;
        std::unordered_map<namespaced_name, id_type, namespaced_name::hash_function> m_name_map;
        std::unordered_map<id_type, namespaced_name>  m_id_map;
    };

    // A registry is a set of object registers keyed off of the type that the register stores.
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
                auto register_ = ref<object_register<T>>(new object_register<T>);
                this->m_registers.insert({ hash_code, register_ });
                return *register_;
            } else {
                return *(ref<object_register<T>>)this->m_registers[hash_code];
            }
        }
    private:
        registry() { }
        std::unordered_map<size_t, ref<ref_counted>> m_registers;
    };
}