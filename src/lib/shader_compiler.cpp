#include "bve_pch.h"
#include "shader_compiler.h"
#include <shaderc/shaderc.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
namespace bve {
    static std::vector<uint32_t> compile_glsl(const std::string& source, shader_type type) {
        // we should probably cache shaders
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
        const bool optimize = false; // we need to change this in release mode!
        if (optimize) {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }
        shaderc_shader_kind kind;
        switch (type) {
        case shader_type::VERTEX:
            kind = shaderc_glsl_vertex_shader;
            break;
        case shader_type::FRAGMENT:
            kind = shaderc_glsl_fragment_shader;
            break;
        case shader_type::GEOMETRY:
            kind = shaderc_glsl_geometry_shader;
            break;
        default:
            throw std::runtime_error("[shader compiler] the specified shader type is not supported yet");
        }
        auto result = compiler.CompileGlslToSpv(source, kind, "not provided");
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("[shader compiler] could not compile shader: " + result.GetErrorMessage());
        }
        return std::vector<uint32_t>(result.cbegin(), result.cend());
    }
    static std::string decompile_glsl(const std::vector<uint32_t>& spirv) {
        spirv_cross::CompilerGLSL compiler(std::move(spirv));
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        for (auto& resource : resources.sampled_images) {
            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            compiler.unset_decoration(resource.id, spv::DecorationDescriptorSet);
            compiler.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
        }
        spirv_cross::CompilerGLSL::Options options;
        options.version = 330;
        options.es = false; // for now
        compiler.set_common_options(options);
        return compiler.compile();
    }
    std::vector<uint32_t> shader_compiler::compile(const std::string& source, shader_language input_language, shader_type type) {
        std::function<std::vector<uint32_t>(const std::string&, shader_type)> compile;
        switch (input_language) {
        case shader_language::GLSL:
            compile = compile_glsl;
            break;
        default:
            throw std::runtime_error("[shader compiler] cannot compile shaders of this language");
        }
        return compile(source, type);
    }
    std::string shader_compiler::decompile(const std::vector<uint32_t>& spirv, shader_language output_language) {
        std::function<std::string(const std::vector<uint32_t>&)> decompile;
        switch (output_language) {
        case shader_language::GLSL:
            decompile = decompile_glsl;
            break;
        default:
            throw std::runtime_error("[shader compiler] cannot decompile to this shader language");
        }
        return decompile(spirv);
    }
    std::string shader_compiler::convert(const std::string& source, shader_language input_language, shader_language output_language, shader_type type) {
        std::vector<uint32_t> spirv = this->compile(source, input_language, type);
        return this->decompile(spirv, output_language);
    }
}