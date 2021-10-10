#pragma once
namespace bve {
    namespace util {
        inline void zero(void* data, size_t size) {
            ::memset(data, 0, size);
        }
        template<typename T> inline void zero(T& data) {
            ::bve::util::zero(&data, sizeof(T));
        }
    }
}