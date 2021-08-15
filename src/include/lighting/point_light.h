#pragma once
#include "light.h"
namespace bve {
    namespace lighting {
        class point_light : public light {
        public:
            point_light() = default;
            point_light(const point_light&) = delete;
            point_light& operator=(const point_light&) = delete;
            void set_constant(float constant);
            void set_linear(float linear);
            void set_quadratic(float quadratic);
            virtual void set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) override;
            virtual light_type get_type() override;
        private:
            float m_constant, m_linear, m_quadratic;
        };
    }
}