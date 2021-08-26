add_custom_target(managed-code ALL
    COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/managed-code
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/managed-code/*.dll ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS basic-voxel-engine
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})