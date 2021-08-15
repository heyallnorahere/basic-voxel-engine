#pragma once
#include "light.h"
namespace bve {
    namespace lighting {
        class spotlight : public light {
        public:
            spotlight() = default;
            spotlight(const spotlight&) = delete;
            spotlight& operator=(const spotlight&) = delete;
            void set_position(glm::vec3 position);
            void set_direction(glm::vec3 direction);
            void set_cutoff(float cutoff);
            virtual void set_uniforms(ref<graphics::shader> shader, const std::string& uniform_name) override;
            virtual light_type get_type() override;
        private:
            glm::vec3 m_position, m_direction;
            float m_cutoff;
        };
    }
}