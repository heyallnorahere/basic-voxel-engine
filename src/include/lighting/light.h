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
            struct uniform_data {
                int32_t type;
                glm::vec3 position, color;
                float ambient_strength, specular_strength;
                // spotlight
                glm::vec3 direction;
                float cutoff;
                // point light
                float constant, linear_, quadratic;
            };
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
            uniform_data get_uniform_data() {
                uniform_data data;
                data.type = (int32_t)this->get_type();
                data.color = this->m_color;
                data.ambient_strength = this->m_ambient_strength;
                data.specular_strength = this->m_specular_strength;
                this->get_uniform_data(data);
                return data;
            }
            virtual light_type get_type() = 0;
        protected:
            virtual void get_uniform_data(uniform_data& data) { }
        private:
            glm::vec3 m_color;
            float m_ambient_strength, m_specular_strength;
        };
    }
}