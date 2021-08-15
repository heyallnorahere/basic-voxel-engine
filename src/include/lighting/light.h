#pragma once
#include "graphics/shader.h"
namespace bve {
    namespace lighting {
        enum class light_type {
            SPOTLIGHT = (1 << 0),
            POINT_LIGHT = (1 << 1),
            DIRECTIONAL_LIGHT = (1 << 2)
        };
        class light : public ref_counted {
        public:
            light() = default;
            virtual ~light() = default;
            light(const light&) = delete;
            light& operator=(const light&) = delete;
            void set_color(glm::vec3 color) {
                this->m_color = color;
            }
            void set_ambient_strength(float ambient_strength) {
                this->m_ambient_strength = ambient_strength;
            }
            void set_specular_strength(float specular_strength) {
                this->m_specular_strength = specular_strength;
            }
            virtual void set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) = 0;
            virtual light_type get_type() = 0;
        protected:
            void set_universal_values(ref<graphics::shader> shader, const std::string& uniform_name) {
                shader->set_vec3(uniform_name + ".color", this->m_color);
                shader->set_float(uniform_name + ".ambient_strength", this->m_ambient_strength);
                shader->set_float(uniform_name + ".specular_strength", this->m_specular_strength);
            }
        private:
            glm::vec3 m_color;
            float m_ambient_strength, m_specular_strength;
        };
    }
}