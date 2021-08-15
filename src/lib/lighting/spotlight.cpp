#include "bve_pch.h"
#include "lighting/spotlight.h"
namespace bve {
    namespace lighting {
        void spotlight::set_position(glm::vec3 position) {
            this->m_position = position;
        }
        void spotlight::set_direction(glm::vec3 direction) {
            this->m_direction = direction;
        }
        void spotlight::set_color(glm::vec3 color) {
            this->m_color = color;
        }
        void spotlight::set_cutoff(float cutoff) {
            this->m_cutoff = cutoff;
        }
        void spotlight::set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) {
            shader->set_vec3(uniform_name + ".position", this->m_position);
            shader->set_vec3(uniform_name + ".direction", this->m_direction);
            shader->set_vec3(uniform_name + ".color", this->m_color);
            shader->set_float(uniform_name + ".cutoff", this->m_cutoff);
            // todo: light type
        }
    }
}