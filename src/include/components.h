#pragma once
#include "script.h"
#include "world.h"
namespace bve {
    namespace components {
        struct transform_component {
            glm::vec3 translation = glm::vec3(0.f);
            glm::vec3 rotation = glm::vec3(0.f);
            glm::vec3 scale = glm::vec3(1.f);
            transform_component() = default;
            transform_component(const transform_component&) = default;
            transform_component(const glm::vec3& translation) {
                this->translation = translation;
            }
            transform_component& operator=(const transform_component&) = default;
            glm::mat4 get_matrix() {
                glm::mat4 rotation = glm::toMat4(glm::quat(this->rotation));
                return glm::translate(glm::mat4(1.f), this->translation) * rotation * glm::scale(glm::mat4(1.f), this->scale);
            }
        };
        struct camera_component {
            glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
            glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
            bool primary = true;
            float near_plane = 0.1f;
            float far_plane = 100.f;
            camera_component() = default;
            camera_component(const camera_component&) = default;
            camera_component& operator=(const camera_component&) = default;
        };
        struct script_component {
            std::vector<ref<script>> scripts;
            entity parent;
            template<typename T, typename... Args> ref<T> bind(Args&&... args) {
                static_assert(std::is_base_of_v<script, T>, "[script component] the given type is not a script type");
                ref<T> script_ = ref<T>::create(std::forward<Args>(args)...);
                script_->m_entity = this->parent;
                script_->on_attach();
                this->scripts.push_back(script_);
                return script_;
            }
        };
    }
    template<> inline void world::on_component_added<components::script_component>(components::script_component& sc, entity ent) {
        sc.parent = ent;
    }
}