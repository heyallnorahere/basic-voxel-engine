#pragma once
#include "graphics/shader.h"
namespace bve {
    namespace lighting {
        class light : public ref_counted {
        public:
            light() = default;
            virtual ~light() = default;
            light(const light&) = delete;
            light& operator=(const light&) = delete;
            virtual void set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) = 0;
        };
    }
}