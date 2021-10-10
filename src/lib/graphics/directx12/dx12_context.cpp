#include "bve_pch.h"
#include "dx12_util.h"
#include "dx12_context.h"
#include "util.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>
namespace bve {
    namespace graphics {
        namespace dx12 {
            dx12_context::dx12_context(ref<dx12_object_factory> factory) {
                this->m_factory = factory;
            }
            dx12_context::~dx12_context() {
                // todo: clean up
            }
            void dx12_context::clear(glm::vec4 clear_color) {
                // todo: clear screen
            }
            void dx12_context::make_current() {
                // todo: make current
            }
            void dx12_context::draw_indexed(size_t index_count) {
                // todo: draw bound pipeline
            }
            void dx12_context::swap_buffers() {
                // todo: swap buffers
            }
            void dx12_context::setup_glfw() {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            }
            void dx12_context::setup_context() {
                this->enable_debug_layer();
                this->create_device();
            }
            void dx12_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                // todo: recreate swapchain and related objects
            }
            void dx12_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForOther(this->m_window, true);
                // todo: init dx12 imgui backend
            }
            void dx12_context::shutdown_imgui_backends() {
                // todo: shutdown imgui
            }
            void dx12_context::call_imgui_backend_newframe() {
                // todo: call NewFrame()
            }
            void dx12_context::render_imgui_draw_data(ImDrawData* data) {
                // todo: draw
            }
            void dx12_context::enable_debug_layer() {
#ifndef NDEBUG
                ComPtr<ID3D12Debug> debug_interface;
                check_hresult(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)), "[dx12 context] could not get debug interface");
                debug_interface->EnableDebugLayer();
#endif
            }
            constexpr D3D_FEATURE_LEVEL device_min_feature_level = D3D_FEATURE_LEVEL_12_0;
            static ComPtr<IDXGIAdapter4> pick_adapter(bool use_warp) {
                ComPtr<IDXGIFactory4> factory;
                create_dxgi_factory(factory);
                ComPtr<IDXGIAdapter1> adapter_1;
                if (use_warp) {
                    check_hresult(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter_1)), "[dx12 context] could not get warp adapter");
                } else {
                    size_t max_dedicated_video_memory = 0;
                    ComPtr<IDXGIAdapter1> iteration_adapter;
                    for (uint32_t i = 0; factory->EnumAdapters1(i, &iteration_adapter) != DXGI_ERROR_NOT_FOUND; i++) {
                        DXGI_ADAPTER_DESC1 adapter_desc;
                        iteration_adapter->GetDesc1(&adapter_desc);
                        if ((adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                            SUCCEEDED(D3D12CreateDevice(iteration_adapter.Get(), device_min_feature_level, __uuidof(ID3D12Device), nullptr)) &&
                            max_dedicated_video_memory < (size_t)adapter_desc.DedicatedVideoMemory) {
                            max_dedicated_video_memory = (size_t)adapter_desc.DedicatedVideoMemory;
                            adapter_1 = iteration_adapter;
                        }
                    }
                }
                if (!adapter_1) {
                    throw std::runtime_error("[dx12 context] no gpus with DirectX 12 support were found");
                }
                ComPtr<IDXGIAdapter4> adapter;
                check_hresult(adapter_1.As(&adapter), "[dx12 context] could not cast IDXGIAdapter1 to IDXGIAdapter4");
                return adapter;
            }
            void dx12_context::create_device() {
                constexpr bool use_warp = false; // for now
                auto adapter = pick_adapter(use_warp);
                check_hresult(D3D12CreateDevice(adapter.Get(), device_min_feature_level, IID_PPV_ARGS(&this->m_device)), "[dx12 context] could not create device");
                DXGI_ADAPTER_DESC1 adapter_desc;
                adapter->GetDesc1(&adapter_desc);
                std::string gpu_name;
                for (size_t i = 0; i < 128; i++) {
                    // a little hacky but it works
                    gpu_name.push_back((char)adapter_desc.Description[i]);
                }
                spdlog::info("[dx12 context] chose device: {0}", gpu_name);
#ifndef NDEBUG
                ComPtr<ID3D12InfoQueue> info_queue;
                if (SUCCEEDED(this->m_device.As(&info_queue))) {
                    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
                    std::vector<D3D12_MESSAGE_ID> deny_ids = {
                        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                        D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                        D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
                    };
                    D3D12_INFO_QUEUE_FILTER filter;
                    util::zero(filter);
                    filter.DenyList.NumIDs = (uint32_t)deny_ids.size();
                    filter.DenyList.pIDList = deny_ids.data();
                    check_hresult(info_queue->PushStorageFilter(&filter), "[dx12 context] could not push storage filter to info queue");
                }
#endif
            }
        }
    }
}