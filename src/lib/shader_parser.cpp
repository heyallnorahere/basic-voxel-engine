#include "bve_pch.h"
#include "shader_parser.h"
namespace bve {
    static std::string read_file(const std::filesystem::path& path) {
        std::ifstream file = std::ifstream(path);
        std::stringstream contents;
        std::string line;
        while (std::getline(file, line)) {
            contents << line << '\n';
        }
        file.close();
        return contents.str();
    }
    static std::map<std::string, shader_type> shader_type_map = {
        { "vertex", shader_type::VERTEX },
        { "fragment", shader_type::FRAGMENT },
        { "geometry", shader_type::GEOMETRY }
    };
    shader_parser::shader_parser(shader_language input, shader_language output) {
        this->m_input_language = input;
        this->m_output_language = output;
    }
    void shader_parser::parse(const std::filesystem::path& path) {
        std::string source = read_file(path);
        this->parse(source, path);
    }
    void shader_parser::parse(const std::string& source, std::optional<std::filesystem::path> path) {
        std::string source_copy = source;
        if (this->m_input_language != shader_language::GLSL) {
            // todo: convert shader to GLSL
        }
        this->process_file(source_copy, path);
    }
    void shader_parser::process_file(const std::string& source, std::optional<std::filesystem::path> path) {
        std::stringstream file(source);
        std::map<shader_type, std::stringstream> sources;
        std::string line, shader_declaration = "#type";
        std::optional<shader_type> current_shader_type;
        while (std::getline(file, line)) {
            if (line.find(shader_declaration) == 0) {
                std::string shader_type_ = line.substr(shader_declaration.length() + 1);
                for (char& c : shader_type_) {
                    c = (char)tolower((int32_t)c);
                }
                current_shader_type = shader_type_map[shader_type_];
            } else {
                if (!current_shader_type) {
                    throw std::runtime_error("[shader parser] the source file did not select a shader");
                }
                sources[*current_shader_type] << line << "\n";
            }
        }
        for (const auto& shader_source : sources) {
            std::string source_ = shader_source.second.str();
            this->process_source(source_, path, shader_source.first);
        }
    }
    class shader_parser_internals {
    public:
        static void include(std::string& source, std::optional<std::filesystem::path> path, shader_type type, const std::list<std::string>& arguments, shader_parser* parser) {
            std::filesystem::path included_path, argument_path;
            for (const auto& arg : arguments) {
                if (arg == *arguments.begin()) {
                    continue;
                }
                if (!arg.empty()) {
                    argument_path = arg;
                }
            }
            if (argument_path.empty()) {
                throw std::runtime_error("[shader parser] include statement does not have a path");
            }
            if (argument_path.is_absolute()) {
                included_path = argument_path;
            } else {
                if (path) {
                    included_path = path->parent_path() / argument_path;
                } else {
                    // if theres no shader path provided we just give up lol
                    included_path = argument_path;
                }
            }
            std::string included_source = read_file(included_path);
            // kind of hacky, but it works, as the shader_parser is going to overwrite it anyway
            parser->process_source(included_source, included_path, type);
            std::string parsed_source = parser->m_sources[type].first;
            source += parsed_source;
        }
    };
    static std::map<std::string, std::function<void(std::string&, std::optional<std::filesystem::path>, shader_type type, const std::list<std::string>&, shader_parser*)>> preprocessor_functions = {
        { "include", shader_parser_internals::include }
    };
    void shader_parser::process_source(const std::string& source, std::optional<std::filesystem::path> path, shader_type type) {
        std::stringstream source_stream(source);
        std::string line, parsed_source;
        while (std::getline(source_stream, line)) {
            if (line.find('#') == 0 && line.length() > 1) {
                std::string preprocessor_command = line.substr(1);
                std::list<std::string> arguments;
                size_t position, last_position = 0;
                bool finished = false;
                while (!finished) {
                    position = preprocessor_command.find_first_of(" \n\"", last_position);
                    if (position == std::string::npos) {
                        finished = true;
                    }
                    std::string arg = preprocessor_command.substr(last_position, finished ? position : (position - last_position));
                    arguments.push_back(arg);
                    last_position = position + 1;
                }
                std::string function_name = *arguments.begin();
                if (preprocessor_functions.find(function_name) == preprocessor_functions.end()) {
                    parsed_source += line + "\n";
                    continue;
                }
                preprocessor_functions[function_name](parsed_source, path, type, arguments, this);
            } else {
                parsed_source += line + "\n";
            }
        }
        auto& pair = this->m_sources[type];
        pair.first = parsed_source;
        pair.second = path;
    }
    std::vector<shader_type> shader_parser::get_parsed_shader_types() {
        std::vector<shader_type> types;
        for (const auto& pair : this->m_sources) {
            types.push_back(pair.first);
        }
        return types;
    }
    std::string shader_parser::get_shader(shader_type type) {
        if (this->m_sources.find(type) == this->m_sources.end()) {
            throw std::runtime_error("[shader parser] a shader of this type was not parsed");
        }
        std::string source = this->m_sources[type].first;
        if (this->m_output_language != shader_language::GLSL) {
            // todo: convert shader source to something like HLSL
        }
        return source;
    }
    std::optional<std::filesystem::path> shader_parser::get_shader_path(shader_type type) {
        if (this->m_sources.find(type) == this->m_sources.end()) {
            throw std::runtime_error("[shader parser] a shader of this type was not parsed");
        }
        return this->m_sources[type].second;
    }
}