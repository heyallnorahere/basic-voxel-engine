if(WIN32)
    include_external_msproject(BasicVoxelEngine ${CMAKE_CURRENT_SOURCE_DIR}/BasicVoxelEngine/BasicVoxelEngine.csproj)
    include_external_msproject(BasicVoxelEngine.Content ${CMAKE_CURRENT_SOURCE_DIR}/BasicVoxelEngine.Content/BasicVoxelEngine.Content.csproj)
    set(MANAGED_TARGET_NAMES BasicVoxelEngine BasicVoxelEngine.Content)
else()
    add_custom_target(managed-code ALL
        COMMAND msbuild -v:q
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(MANAGED_TARGET_NAMES managed-code)
endif()
set_target_properties(${MANAGED_TARGET_NAMES} PROPERTIES FOLDER BVE)