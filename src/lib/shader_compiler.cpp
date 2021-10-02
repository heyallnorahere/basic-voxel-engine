#include "bve_pch.h"
#include "shader_compiler.h"
#include <StandAlone/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <SPIRV/GlslangToSpv.h>
#include <spirv-tools/optimizer.hpp>
namespace bve {
    void shader_compiler::initialize_compiler() {
        if (!glslang::InitializeProcess()) {
            throw std::runtime_error("[shader compiler] could not initialize glslang!");
        }
    }
    void shader_compiler::cleanup_compiler() {
        glslang::FinalizeProcess();
    }
    struct compiler_options {
        glslang::EShSource language;
        glslang::EShClient client;
        glslang::EShTargetClientVersion client_version;
    };
    static void throw_compilation_error(const std::string& message, const std::string& shader_name) {
        throw std::runtime_error("[shader compiler] could not compile " + shader_name + " shader: " + message);
    }
    static std::vector<uint32_t> compile(const std::string& source, shader_type type, const compiler_options& options) {
        // we should probably cache shaders
        EShLanguage shader_type;
        std::string shader_name;
        switch (type) {
        case shader_type::VERTEX:
            shader_type = EShLangVertex;
            shader_name = "vertex";
            break;
        case shader_type::FRAGMENT:
            shader_type = EShLangFragment;
            shader_name = "fragment";
            break;
        case shader_type::GEOMETRY:
            shader_type = EShLangGeometry;
            shader_name = "geometry";
            break;
        default:
            throw std::runtime_error("[shader compiler] the specified shader type is not supported yet");
        }
        auto program = std::make_shared<glslang::TProgram>();
        auto shader = std::make_shared<glslang::TShader>(shader_type);
        const char* src = source.c_str();
        shader->setStrings(&src, 1);
        glslang::EShTargetLanguage target_language = glslang::EShTargetSpv;
        glslang::EShTargetLanguageVersion target_language_version;
        switch (options.client_version) {
        case glslang::EShTargetVulkan_1_0:
            target_language_version = glslang::EShTargetSpv_1_0;
            break;
        case glslang::EShTargetVulkan_1_1:
            target_language_version = glslang::EShTargetSpv_1_3;
            break;
        case glslang::EShTargetVulkan_1_2:
            target_language_version = glslang::EShTargetSpv_1_5;
            break;
        case glslang::EShTargetOpenGL_450:
            target_language_version = glslang::EShTargetSpv_1_0;
            break;
        default:
            throw std::runtime_error("[shader compiler] invalid client version");
        }
        shader->setEnvTarget(target_language, target_language_version);
        shader->setEnvInput(options.language, shader_type, options.client, options.client_version);
        shader->setEnvClient(options.client, options.client_version);
        shader->setEntryPoint("main");
        EShMessages messages = EShMsgDefault;
        if (options.language == glslang::EShSourceHlsl) {
            (uint32_t&)messages |= EShMsgReadHlsl;
        }
        if (!shader->parse(&glslang::DefaultTBuiltInResource, 330, true, messages)) throw_compilation_error(shader->getInfoLog(), shader_name);
        program->addShader(shader.get());
        if (!program->link(messages)) throw_compilation_error(shader->getInfoLog(), shader_name);
        auto intermediate = program->getIntermediate(shader_type);
        if (intermediate) {
            std::vector<uint32_t> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spirv_options;
            constexpr bool optimize = false; // we need to change this in release mode!
            if (optimize) {
                spirv_options.optimizeSize = true;
                spirv_options.stripDebugInfo = true;
            } else {
                spirv_options.disableOptimizer = true;
                spirv_options.generateDebugInfo = true;
            }
            spirv_options.disassemble = false;
            spirv_options.validate = true;
            glslang::GlslangToSpv(*intermediate, spirv, &spirv_options);
            return spirv;
        } else {
            return { };
        }
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
        options.version = 410;
        options.es = false;
        return decompile(spirv, options);
    }
    static std::string decompile_hlsl(const std::vector<uint32_t>& spirv) {
        spirv_cross::CompilerGLSL::Options options;
        // todo: set options, if necessary
        return decompile<spirv_cross::CompilerHLSL>(spirv, options);
    }
    std::vector<uint32_t> shader_compiler::compile(const std::string& source, shader_language input_language, shader_type type) {
        compiler_options options;
        switch (input_language) {
        case shader_language::GLSL:
            options.client = glslang::EShClientVulkan;
            options.client_version = glslang::EShTargetVulkan_1_0; // we're using vulkan 1.0 in graphics/vulkan/vulkan_context.cpp
            options.language = glslang::EShSourceGlsl;
            break;
        case shader_language::OpenGLGLSL:
            options.client = glslang::EShClientOpenGL;
            options.client_version = glslang::EShTargetOpenGL_450;
            options.language = glslang::EShSourceGlsl;
            break;
        case shader_language::HLSL:
            options.client = glslang::EShClientVulkan;
            options.client_version = glslang::EShTargetVulkan_1_0;
            options.language = glslang::EShSourceHlsl;
            break;
        default:
            throw std::runtime_error("[shader compiler] cannot compile shaders of this language");
        }
        auto spirv = ::bve::compile(source, type, options);
        if (spirv.empty()) {
            throw std::runtime_error("[shader compiler] no spirv was returned!");
        }
        return spirv;
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
    static std::map<spirv_cross::TypeID, std::shared_ptr<graphics::struct_data>> defined_structs;
    static size_t get_size(const spirv_cross::SPIRType& type, const spirv_cross::Compiler& compiler) {
        using BaseType = spirv_cross::SPIRType::BaseType;
        switch (type.basetype) {
        case BaseType::Boolean:
        case BaseType::Char:
            return 1;
        case BaseType::Float:
        case BaseType::Int:
        case BaseType::UInt:
            return 4;
        case BaseType::Int64:
        case BaseType::Double:
        case BaseType::UInt64:
            return 8;
        case BaseType::Struct:
            return compiler.get_declared_struct_size(type);
        default:
            throw std::runtime_error("[shader compiler] invalid base type");
        }
        return 0;
    }
    static std::shared_ptr<graphics::struct_data> get_type(const spirv_cross::Compiler& compiler, spirv_cross::TypeID id, spirv_cross::TypeID parent, size_t member_index) {
        if (defined_structs.find(id) != defined_structs.end()) {
            return defined_structs[id];
        }
        auto data = std::make_shared<graphics::struct_data>();
        defined_structs.insert({ id, data });
        const auto& spirv_type = compiler.get_type(id);
        data->name = compiler.get_name(id);
        data->size = get_size(spirv_type, compiler);
        if (spirv_type.columns > 1) {
            data->size *= (size_t)spirv_type.columns;
        }
        if (spirv_type.array.empty()) {
            data->array_size = 1;
            data->array_stride = 0;
        } else {
            data->array_size = spirv_type.array[0];
            const auto& parent_type = compiler.get_type(parent);
            data->array_stride = compiler.type_struct_member_array_stride(parent_type, member_index);
        }
        for (size_t i = 0; i < spirv_type.member_types.size(); i++) {
            std::string name = compiler.get_member_name(spirv_type.self, (uint32_t)i);
            graphics::field_data field;
            field.type = get_type(compiler, spirv_type.member_types[i], id, i).get();
            field.offset = compiler.type_struct_member_offset(spirv_type, i);
            data->fields.insert({ name, field });
        }
        return data;
    }
    void shader_compiler::reflect(const std::vector<uint32_t>& spirv, graphics::reflection_output& output) {
        spirv_cross::Compiler compiler(spirv);
        auto resources = compiler.get_shader_resources();
        for (const auto& resource : resources.uniform_buffers) {
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            graphics::uniform_buffer_data& ubd = output.uniform_buffers[binding];
            ubd.name = resource.name;
            ubd.type = get_type(compiler, resource.base_type_id, spirv_cross::TypeID(), 0);
        }
        for (const auto& [_, struct_] : defined_structs) {
            output.structs.push_back(struct_);
        }
        defined_structs.clear();
    }
}