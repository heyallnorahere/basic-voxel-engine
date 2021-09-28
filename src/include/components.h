#pragma once
#include "code_host.h"
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
            struct script {
                ref<managed::object> script_object;
                ref<managed::class_> script_type;
                void update() {
                    MonoMethod* method = this->script_type->get_method("*:Update()");
                    if (!method) {
                        return;
                    }
                    this->script_object->invoke(method);
                }
            };
            script_component(ref<code_host> host) {
                this->host = host;
                this->base_class = this->host->find_class("BasicVoxelEngine.Script");
                if (!this->base_class) {
                    throw std::runtime_error("[script component] the BasicVoxelEngine.Script class does not exist in the given host's domain");
                }
            }
            std::vector<script> scripts;
            entity parent;
            ref<code_host> host;
            ref<managed::class_> base_class;
            template<typename... Args> script& bind(ref<managed::class_> script_type, Args*&&... args) {
                auto& sc = scripts.emplace_back();
                sc.script_type = script_type;
                if (!sc.script_type || !sc.script_type->get()) {
                    throw std::runtime_error("[script component] nullptr was passed");
                }
                if (!sc.script_type->derives_from(this->base_class)) {
                    throw std::runtime_error("[script component] the given type must derive from BasicVoxelEngine.Script");
                }
                sc.script_object = sc.script_type->instantiate(std::forward<Args*>(args)...);
                return sc;
            }
            template<typename... Args> script& bind(const std::string& script_name, Args*&&... args) {
                auto script_type = this->host->find_class(script_name);
                if (script_type) {
                    throw std::runtime_error("[script component] no such class found");
                }
                return this->bind(script_type, std::forward<Args*>(args)...);
            }
        };
    }
    template<> inline void world::on_component_added<components::script_component>(components::script_component& sc, entity ent) {
        sc.parent = ent;
    }
}