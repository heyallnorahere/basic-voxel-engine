# adapted from https://github.com/Microsoft/DirectXShaderCompiler/blob/master/cmake/modules/FindD3D12.cmake
# Find the Win10 SDK path.
if ("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "" )
  get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
  set (WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
elseif(TRUE)
  set (WIN10_SDK_PATH $ENV{WIN10_SDK_PATH})
  set (WIN10_SDK_VERSION $ENV{WIN10_SDK_VERSION})
endif ("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "" )

# WIN10_SDK_PATH will be something like C:\Program Files (x86)\Windows Kits\10
# WIN10_SDK_VERSION will be something like 10.0.14393 or 10.0.14393.0; we need the
# one that matches the directory name.

if (IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")
  set(WIN10_SDK_VERSION "${WIN10_SDK_VERSION}.0")
endif (IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")


# Find the d3d12 and dxgi include path, it will typically look something like this.
# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\um\d3d12.h
# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\shared\dxgi1_4.h
find_path(D3D12_INCLUDE_DIR    # Set variable D3D12_INCLUDE_DIR
          d3d12.h                # Find a path with d3d12.h
          HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/um"
          DOC "path to WIN10 SDK header files"
          HINTS
          )

find_path(DXGI_INCLUDE_DIR    # Set variable DXGI_INCLUDE_DIR
          dxgi1_4.h           # Find a path with dxgi1_4.h
          HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/shared"
          DOC "path to WIN10 SDK header files"
          HINTS
          )
set(D3D12_INCLUDE_DIRS ${D3D12_INCLUDE_DIR} ${DXGI_INCLUDE_DIR})

# Find D3D libraries
set(D3D12_LIB_NAMES d3d12.lib dxgi.lib d3dcompiler.lib)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(D3D12_SDK_SUFFIX x64)
else()
    set(D3D12_SDK_SUFFIX x86)
endif()
set(D3D12_HINTS_PATH ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}/um/${D3D12_SDK_SUFFIX})

set(D3D12_LIBRARIES)
foreach (D3D12_LIB_NAME ${D3D12_LIB_NAMES})
  find_library(${D3D12_LIB_NAME}_LOC NAMES ${D3D12_LIB_NAME} HINTS ${D3D12_HINTS_PATH})
  set(D3D12_LIBRARIES ${D3D12_LIBRARIES} ${${D3D12_LIB_NAME}_LOC})
endforeach(D3D12_LIB_NAME)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set D3D12_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(D3D12  DEFAULT_MSG
                                  D3D12_INCLUDE_DIRS D3D12_LIBRARIES)

mark_as_advanced(D3D12_INCLUDE_DIRS D3D12_LIBRARIES)
