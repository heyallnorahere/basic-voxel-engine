#pragma once
namespace bve {
    class buffer {
    public:
        buffer();
        buffer(size_t size);
        buffer(const void* data, size_t size, bool copy = true);
        template<typename T> buffer(const T& data) : buffer(&data, sizeof(T)) { }
        buffer(const buffer& buf);
        buffer& operator=(const buffer& buf);
        ~buffer();
        void alloc(size_t size);
        void free();
        void zero();
        void copy(const void* data, size_t size, size_t offset = 0);
        template<typename T> void copy(const T& data) {
            copy(&data, sizeof(T));
        }
        void copy(const buffer& buf);
        template<typename T> T* get() {
            return (T*)this->m_pointer;
        }
        template<typename T> const T* get() const {
            return (const T*)this->m_pointer;
        }
        size_t size() const;
        operator bool() const;
        template<typename T> operator T*() {
            return this->get<T>();
        }
        template<typename T> operator const T*() const {
            return this->get<T>();
        }
        template<typename T> operator T&() {
            if (!this->m_pointer) {
                throw std::runtime_error("[buffer] tried to access unallocated memory");
            }
            return *this->get<T>();
        }
        template<typename T> operator const T&() {
            if (!this->m_pointer) {
                throw std::runtime_error("[buffer] tried to access unallocated memory");
            }
            return *this->get<T>();
        }
    private:
        void* m_pointer;
        size_t m_size;
    };
}