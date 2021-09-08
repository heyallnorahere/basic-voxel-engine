find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND ${Python3_EXECUTABLE} utils/git-sync-deps WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shaderc")
add_subdirectory("shaderc")