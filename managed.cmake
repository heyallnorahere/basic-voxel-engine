if(WIN32)
    include_external_msproject(BasicVoxelEngine ${CMAKE_CURRENT_SOURCE_DIR}/BasicVoxelEngine/BasicVoxelEngine.csproj)
    set(MANAGED_TARGET_NAMES BasicVoxelEngine)
else()
    add_custom_target(managed-code ALL
        COMMAND dotnet build -o ${CMAKE_CURRENT_BINARY_DIR}/managed-code
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(MANAGED_TARGET_NAMES managed-code)
endif()
set_target_properties(${MANAGED_TARGET_NAMES} PROPERTIES FOLDER BVE)