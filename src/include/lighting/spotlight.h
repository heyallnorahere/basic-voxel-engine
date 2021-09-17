#pragma once
#include "light.h"
namespace bve {
    namespace lighting {
        class spotlight : public light {
        public:
            spotlight() = default;
            spotlight(const spotlight&) = delete;
            spotlight& operator=(const spotlight&) = delete;
            void set_direction(glm::vec3 direction);
            void set_cutoff(float cutoff);
            virtual light_type get_type() override;
        protected:
            virtual void get_uniform_data(uniform_data& data) override;
        private:
            glm::vec3 m_direction;
            float m_cutoff;
        };
    }
}