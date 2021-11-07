#pragma once
namespace bve {
    enum class shader_language {
        GLSL,
        OpenGLGLSL,
        HLSL
    };
    enum class shader_type {
        VERTEX,
        FRAGMENT,
        GEOMETRY,
        COMPUTE
    };
    class shader_parser_internals;
    class shader_parser {
    public:
        static shader_language get_language(const std::vector<fs::path>& paths);
        shader_parser(shader_language input, shader_language output);
        void parse(const fs::path& path);
        void parse(const std::string& source, std::optional<fs::path> path = std::optional<fs::path>());
        std::vector<shader_type> get_parsed_shader_types() const;
        std::string get_shader(shader_type type) const;
        std::optional<fs::path> get_shader_path(shader_type type);
    private:
        void process_file(const std::string& source, std::optional<fs::path> path);
        void process_source(const std::string& source, std::optional<fs::path> path, shader_type type);
        std::unordered_map<shader_type, std::pair<std::string, std::optional<fs::path>>> m_sources;
        shader_language m_input_language, m_output_language;
        friend class shader_parser_internals;
    };
}