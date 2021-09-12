#include "bve_pch.h"
#include "shader_compiler.h"
#include <shaderc/shaderc.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
namespace bve {
    static std::vector<uint32_t> compile(const std::string& source, shader_type type, shaderc::CompileOptions compile_options) {
        // we should probably cache shaders
        shaderc::Compiler compiler;
        shaderc::CompileOptions options = compile_options;
        const bool optimize = false; // we need to change this in release mode!
        if (optimize) {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }
        shaderc_shader_kind kind;
        std::string shader_name;
        switch (type) {
        case shader_type::VERTEX:
            kind = shaderc_glsl_vertex_shader;
            shader_name = "vertex";
            break;
        case shader_type::FRAGMENT:
            kind = shaderc_glsl_fragment_shader;
            shader_name = "fragment";
            break;
        case shader_type::GEOMETRY:
            kind = shaderc_glsl_geometry_shader;
            shader_name = "geometry";
            break;
        default:
            throw std::runtime_error("[shader compiler] the specified shader type is not supported yet");
        }
        auto result = compiler.CompileGlslToSpv(source, kind, "<not found>", "main", options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error("[shader compiler] could not compile " + shader_name + " shader: " + result.GetErrorMessage());
        }
        return std::vector<uint32_t>(result.cbegin(), result.cend());
    }
    template<typename T = spirv_cross::CompilerGLSL> static std::string decompile(const std::vector<uint32_t>& spirv, const spirv_cross::CompilerGLSL::Options& options) {
        std::shared_ptr<spirv_cross::CompilerGLSL> compiler = std::make_shared<T>(std::move(spirv));
        spirv_cross::ShaderResources resources = compiler->get_shader_resources();
        for (auto& resource : resources.sampled_images) {
            uint32_t set = compiler->get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler->get_decoration(resource.id, spv::DecorationBinding);
            compiler->unset_decoration(resource.id, spv::DecorationDescriptorSet);
            compiler->set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
        }
        compiler->set_common_options(options);
        return compiler->compile();
    }
    static std::string decompile_vulkan_glsl(const std::vector<uint32_t>& spirv) {
        spirv_cross::CompilerGLSL::Options options;
        options.vulkan_semantics = true;
        options.version = 460;
        options.es = false; // for now
        return decompile(spirv, options);
    }
    static std::string decompile_opengl_glsl(const std::vector<uint32_t>& spirv) {
        spirv_cross::CompilerGLSL::Options options;
        options.vulkan_semantics = false;
        options.version = 330;
        options.es = false;
        return decompile(spirv, options);
    }
    static std::string decompile_hlsl(const std::vector<uint32_t>& spirv) {
        spirv_cross::CompilerGLSL::Options options;
        // todo: set options, if necessary
        return decompile<spirv_cross::CompilerHLSL>(spirv, options);
    }
    std::vector<uint32_t> shader_compiler::compile(const std::string& source, shader_language input_language, shader_type type) {
        shaderc::CompileOptions options;
        switch (input_language) {
        case shader_language::GLSL:
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
            break;
        case shader_language::OpenGLGLSL:
            options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
            break;
        case shader_language::HLSL:
            options.SetSourceLanguage(shaderc_source_language_hlsl);
            break;
        default:
            throw std::runtime_error("[shader compiler] cannot compile shaders of this language");
        }
        return ::bve::compile(source, type, options);
    }
    std::string shader_compiler::decompile(const std::vector<uint32_t>& spirv, shader_language output_language) {
        std::function<std::string(const std::vector<uint32_t>&)> decompile;
        switch (output_language) {
        case shader_language::GLSL:
            decompile = decompile_vulkan_glsl;
            break;
        case shader_language::OpenGLGLSL:
            decompile = decompile_opengl_glsl;
            break;
        case shader_language::HLSL:
            decompile = decompile_hlsl;
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