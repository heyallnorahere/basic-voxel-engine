#pragma once
namespace bve {
    namespace graphics {
        class shader : public ref_counted {
        public:
            shader() = default;
            virtual ~shader() = default;
            shader(const shader&) = delete;
            shader& operator=(const shader&) = delete;
            virtual void reload() = 0;
            virtual void bind() = 0;
            virtual void unbind() = 0;
            virtual void set_int(const std::string& name, int32_t value) = 0;
            virtual void set_float(const std::string& name, float value) = 0;
            virtual void set_ivec2(const std::string& name, const glm::ivec2& value) = 0;
            virtual void set_ivec3(const std::string& name, const glm::ivec3& value) = 0;
            virtual void set_ivec4(const std::string& name, const glm::ivec4& value) = 0;
            virtual void set_vec2(const std::string& name, const glm::vec2& value) = 0;
            virtual void set_vec3(const std::string& name, const glm::vec3& value) = 0;
            virtual void set_vec4(const std::string& name, const glm::vec4& value) = 0;
            virtual void set_mat4(const std::string& name, const glm::mat4& value) = 0;
        };
    }
}