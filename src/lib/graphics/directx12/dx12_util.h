#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;
#include <initguid.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <D3Dcompiler.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
namespace bve {
    namespace graphics {
        namespace dx12 {
            inline void check_hresult(HRESULT result, const std::string& message) {
                if (FAILED(result)) {
                    throw std::runtime_error(message);
                }
            }
            inline void create_dxgi_factory(ComPtr<IDXGIFactory4>& dxgi_factory) {
                uint32_t creation_flags = 0;
#ifndef NDEBUG
                creation_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
                check_hresult(CreateDXGIFactory2(creation_flags, IID_PPV_ARGS(&dxgi_factory)), "[dx12 utilities] could not create dxgi factory");
            }
        }
    }
}
