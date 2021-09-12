#pragma once
#include "shader_parser.h"
namespace bve {
    class shader_compiler {
    public:
        static void initialize_compiler();
        static void cleanup_compiler();
        std::vector<uint32_t> compile(const std::string& source, shader_language input_language, shader_type type);
        std::string decompile(const std::vector<uint32_t>& spirv, shader_language output_language);
        std::string convert(const std::string& source, shader_language input_language, shader_language output_language, shader_type type);
    };
}