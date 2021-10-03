#pragma once
#include "../shader_parser.h"
namespace bve {
    namespace graphics {
        struct struct_data;
        struct field_data {
            size_t offset;
            struct_data* type;
        };
        struct struct_data {
            std::string name;
            size_t size, array_stride;
            uint32_t array_size;
            std::map<std::string, field_data> fields;
            size_t find_offset(const std::string& field_name);
        };
        struct uniform_buffer_data {
            std::string name;
            std::shared_ptr<struct_data> type;
            shader_type stage;
            uint32_t descriptor_set;
        };
        struct reflection_output {
            std::map<uint32_t, uniform_buffer_data> uniform_buffers;
            std::vector<std::shared_ptr<struct_data>> structs;
        };
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
            virtual int32_t get_int(const std::string& name) = 0;
            virtual float get_float(const std::string& name) = 0;
            virtual glm::ivec2 get_ivec2(const std::string& name) = 0;
            virtual glm::ivec3 get_ivec3(const std::string& name) = 0;
            virtual glm::ivec4 get_ivec4(const std::string& name) = 0;
            virtual glm::vec2 get_vec2(const std::string& name) = 0;
            virtual glm::vec3 get_vec3(const std::string& name) = 0;
            virtual glm::vec4 get_vec4(const std::string& name) = 0;
            virtual glm::mat4 get_mat4(const std::string& name) = 0;
            const reflection_output& get_reflection_data() {
                return this->m_reflection_data;
            }
        protected:
            void reflect(shader_type type, const std::vector<uint32_t>& spirv);
        private:
            reflection_output m_reflection_data;
        };
    }
}