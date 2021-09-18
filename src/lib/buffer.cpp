#include "bve_pch.h"
#include "buffer.h"
namespace bve {
    buffer::buffer() {
        this->m_pointer = nullptr;
        this->m_size = 0;
    }
    buffer::buffer(size_t size) : buffer() {
        this->alloc(size);
    }
    buffer::buffer(const void* data, size_t size, bool copy) : buffer() {
        if (copy) {
            this->copy(data, size);
        } else {
            this->m_pointer = (void*)data;
            this->m_size = size;
        }
    }
    buffer::buffer(const buffer& buf) : buffer() {
        this->copy(buf);
    }
    buffer& buffer::operator=(const buffer& buf) {
        this->copy(buf);
        return *this;
    }
    buffer::~buffer() {
        if (this->m_pointer) {
            this->free();
        }
    }
    void buffer::alloc(size_t size) {
        if (this->m_pointer) {
            this->free();
        }
        this->m_size = size;
        this->m_pointer = malloc(this->m_size);
        spdlog::info("[buffer] allocated {0} (size: {1})", this->m_pointer, this->m_size);
        if (!this->m_pointer) {
            throw std::runtime_error("[buffer] ran out of memory");
        }
    }
    void buffer::free() {
        ::free(this->m_pointer);
        spdlog::info("[buffer] freed {0} (size: {1})", this->m_pointer, this->m_size);
        this->m_pointer = nullptr;
        this->m_size = 0;
    }
    void buffer::zero() {
        if (!this->m_pointer) {
            throw std::runtime_error("[buffer] tried to access unallocated memory");
        }
        memset(this->m_pointer, 0, this->m_size);
    }
    void buffer::copy(const void* data, size_t size, size_t offset) {
        if (!data) {
            throw std::runtime_error("[buffer] tried to access unallocated memory");
        }
        if (!this->m_pointer || this->m_size < size + offset) {
            buffer temp;
            if (this->m_pointer) {
                temp.copy(*this);
            }
            this->alloc(size + offset);
            if (temp) {
                this->copy(temp);
            }
        }
        spdlog::info("[buffer] copying {0} (size: {1}) into {2} (size: {3})", data, size, this->m_pointer, this->m_size);
        void* ptr = (void*)((size_t)this->m_pointer + offset);
        memcpy(ptr, data, this->m_size);
    }
    void buffer::copy(const buffer& buf) {
        this->copy(buf.m_pointer, buf.m_size);
    }
    size_t buffer::size() const {
        return this->m_size;
    }
    buffer::operator bool() const {
        return this->m_pointer;
    }
    void* buffer::operator+(size_t offset) {
        return (void*)((size_t)this->m_pointer + offset);
    }
    const void* buffer::operator+(size_t offset) const {
        return (void*)((size_t)this->m_pointer + offset);
    }
}