class Option:
    def __init__(self, name: str, value: str):
        self.name = name
        self.value = value
    def serialize(self):
        return f"-D{self.name}={self.value}"
OPTIONS = [
    Option("GLFW_BUILD_EXAMPLES", "OFF"),
    Option("GLFW_BUILD_TESTS", "OFF"),
    Option("GLFW_BUILD_DOCS", "OFF"),
    Option("GLFW_INSTALL", "OFF"),
    Option("INJECT_DEBUG_POSTFIX", "OFF"),
    Option("ASSIMP_BUILD_TESTS", "OFF"),
    Option("ASSIMP_BUILD_ASSIMP_TOOLS", "OFF"),
    Option("SPIRV_CROSS_CLI", "OFF"),
    Option("SPIRV_CROSS_ENABLE_TESTS", "OFF"),
    Option("ENABLE_CTEST", "OFF"),
    Option("SKIP_GLSLANG_INSTALL", "ON"),
    Option("ENABLE_HLSL", "ON"),
]
def main():
    output = ""
    for option in OPTIONS:
        if len(output) > 0:
            output += " "
        output += option.serialize()
    print(output)
if __name__ == "__main__":
    main()