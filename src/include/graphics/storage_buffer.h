#pragma once
#include "../buffer.h"
namespace bve {
    namespace graphics {
        class storage_buffer : public ref_counted {
        public:
            virtual ~storage_buffer() = default;
            virtual void set_data(const void* data, size_t size, size_t offset = 0) = 0;
            virtual void get_data(void* data, size_t size, size_t offset = 0) = 0;
            virtual void activate() = 0;
            void set_data(const ::bve::buffer& buf, size_t offset = 0) {
                this->set_data(buf.get<void>(), buf.size(), offset);
            }
            void get_data(::bve::buffer& buf, size_t offset = 0) {
                this->get_data(buf.get<void>(), buf.size(), offset);
            }
            template<typename T> void set_data(const T& data, size_t offset = 0) {
                this->set_data(&data, sizeof(T), offset);
            }
            template<typename T> void get_data(T& data, size_t offset = 0) {
                this->get_data(&data, sizeof(T), offset);
            }
        };
    }
}