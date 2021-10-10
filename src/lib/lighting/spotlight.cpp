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
        light_type spotlight::get_type() {
            return light_type::SPOTLIGHT;
        }
        void spotlight::get_uniform_data(uniform_data& data) {
            data.direction = this->m_direction;
            data.cutoff = this->m_cutoff;
        }
    }
}