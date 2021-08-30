add_custom_target(managed-code ALL
    COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/managed-code
    DEPENDS basic-voxel-engine
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})