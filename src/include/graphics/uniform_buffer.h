#pragma once
#include "../buffer.h"
namespace bve {
    namespace graphics {
        class uniform_buffer : public ref_counted {
        public:
            virtual ~uniform_buffer() { }
            virtual void set_data(const void* data, size_t size, size_t offset = 0) = 0;
            void set_data(const ::bve::buffer& buf, size_t offset = 0) {
                this->set_data(buf.get<void>(), buf.size(), offset);
            }
            template<typename T> void set_data(const T& data, size_t offset = 0) {
                this->set_data(&data, sizeof(T), offset);
            }
        };
    }
}