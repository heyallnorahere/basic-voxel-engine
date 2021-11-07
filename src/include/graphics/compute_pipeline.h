#pragma once
#include "uniform_buffer.h"
namespace bve {
    namespace graphics {
        class compute_pipeline : public ref_counted {
        public:
            virtual ~compute_pipeline() = default;
            virtual void bind_uniform_buffer(ref<uniform_buffer> buffer) = 0;
            virtual void dispatch(glm::uvec3 group_count = glm::uvec3(1)) = 0;
        };
    }
}