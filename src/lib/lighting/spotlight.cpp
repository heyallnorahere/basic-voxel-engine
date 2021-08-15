#include "bve_pch.h"
#include "lighting/spotlight.h"
namespace bve {
    namespace lighting {
        void spotlight::set_direction(glm::vec3 direction) {
            this->m_direction = direction;
        }
        void spotlight::set_cutoff(float cutoff) {
            this->m_cutoff = cutoff;
        }
        void spotlight::set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) {
            this->set_universal_values(shader, uniform_name);
            shader->set_vec3(uniform_name + ".direction", this->m_direction);
            shader->set_float(uniform_name + ".cutoff", this->m_cutoff);
        }
        light_type spotlight::get_type() {
            return light_type::SPOTLIGHT;
        }
    }
}