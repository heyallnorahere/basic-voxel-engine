class Option:
    def __init__(self, name: str, value: str):
        self.name = name
        self.value = value
    def serialize(self):
        return f"-D{self.name}={self.value}"
OPTIONS = {
    "GLFW_BUILD_EXAMPLES": "OFF",
    "GLFW_BUILD_TESTS": "OFF",
    "GLFW_BUILD_DOCS": "OFF",
    "GLFW_INSTALL": "OFF",
    "INJECT_DEBUG_POSTFIX": "OFF",
    "ASSIMP_BUILD_TESTS": "OFF",
    "ASSIMP_BUILD_ASSIMP_TOOLS": "OFF",
    "SPIRV_CROSS_CLI": "OFF",
    "SPIRV_CROSS_ENABLE_TESTS": "OFF",
    "ENABLE_CTEST": "OFF",
    "SKIP_GLSLANG_INSTALL": "ON",
    "ENABLE_HLSL": "ON",
    "SKIP_SPIRV_TOOLS_INSTALL": "ON",
    "SPIRV_TOOLS_BUILD_STATIC": "ON",
    "SPIRV_SKIP_EXECUTABLES": "ON",
}
def main():
    output = ""
    for name in OPTIONS.keys():
        if len(output) > 0:
            output += " "
        option = Option(name, OPTIONS[name])
        output += option.serialize()
    print(output)
if __name__ == "__main__":
    main()