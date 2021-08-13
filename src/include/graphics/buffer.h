#pragma once
namespace bve {
    namespace graphics {
        class buffer : public ref_counted {
        public:
            buffer() = default;
            virtual ~buffer() = default;
            buffer(const buffer&) = delete;
            buffer& operator=(const buffer&) = delete;
            virtual void bind() = 0;
            virtual void unbind() = 0;
        };
    }
}