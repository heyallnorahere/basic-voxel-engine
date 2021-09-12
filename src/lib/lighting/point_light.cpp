#include "bve_pch.h"
#include "lighting/point_light.h"
namespace bve {
    namespace lighting {
        void point_light::set_constant(float constant) {
            this->m_constant = constant;
        }
        void point_light::set_linear(float linear) {
            this->m_linear = linear;
        }
        void point_light::set_quadratic(float quadratic) {
            this->m_quadratic = quadratic;
        }
        void point_light::set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) {
            this->set_universal_values(shader, uniform_name);
            shader->set_float(uniform_name + ".constant", this->m_constant);
            shader->set_float(uniform_name + ".linear_", this->m_linear);
            shader->set_float(uniform_name + ".quadratic", this->m_quadratic);
        }
        light_type point_light::get_type() {
            return light_type::POINT_LIGHT;
        }
    }
}