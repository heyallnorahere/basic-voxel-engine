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
        light_type point_light::get_type() {
            return light_type::POINT_LIGHT;
        }
        void point_light::get_uniform_data(uniform_data& data) {
            data.constant = this->m_constant;
            data.linear_ = this->m_linear;
            data.quadratic = this->m_quadratic;
        }
    }
}