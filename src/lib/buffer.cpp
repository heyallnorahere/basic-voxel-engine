#include "bve_pch.h"
#include "buffer.h"
#include "util.h"
namespace bve {
    buffer::buffer() {
        this->m_pointer = nullptr;
        this->m_size = 0;
        this->m_dynamic_resizing_enabled = true;
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
    void buffer::set_dynamic_resizing_enabled(bool enabled) {
        this->m_dynamic_resizing_enabled = enabled;
    }
    void buffer::alloc(size_t size) {
        if (this->m_pointer) {
            this->free();
        }
        this->m_size = size;
        this->m_pointer = malloc(this->m_size);
#ifdef BVE_BUFFER_TRACE
        spdlog::info("[buffer] allocated {0} (size: {1})", this->m_pointer, this->m_size);
#endif
        if (!this->m_pointer) {
            throw std::runtime_error("[buffer] ran out of memory");
        }
    }
    void buffer::free() {
        ::free(this->m_pointer);
#ifdef BVE_BUFFER_TRACE
        spdlog::info("[buffer] freed {0} (size: {1})", this->m_pointer, this->m_size);
#endif
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
            if (this->m_dynamic_resizing_enabled) {
                buffer temp;
                if (this->m_pointer) {
                    temp.copy(*this);
                }
                this->alloc(size + offset);
                if (temp.m_pointer) {
                    this->copy(temp);
                }
            } else {
                throw std::runtime_error("[buffer] tried to copy into unallocated memory");
            }
        }
#ifdef BVE_BUFFER_TRACE
        std::stringstream address_string;
        address_string << this->m_pointer;
        if (offset > 0) {
            address_string << "+0x" << std::hex << offset;
        }
        spdlog::info("[buffer] copying {0} (size: {1}) into {2} (size: {3})", data, size, address_string.str(), this->m_size - offset);
#endif
        void* ptr = (void*)((size_t)this->m_pointer + offset);
        memcpy(ptr, data, size);
    }
    void buffer::copy(const buffer& buf, size_t offset) {
        this->copy(buf.m_pointer, buf.m_size, offset);
    }
    size_t buffer::size() const {
        return this->m_size;
    }
    void* buffer::operator+(size_t offset) {
        return (void*)((size_t)this->m_pointer + offset);
    }
    const void* buffer::operator+(size_t offset) const {
        return (void*)((size_t)this->m_pointer + offset);
    }
}