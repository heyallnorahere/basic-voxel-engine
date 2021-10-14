#include "bve_pch.h"
#include "dx12_util.h"
#include "dx12_context.h"
#include "util.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
namespace bve {
    namespace graphics {
        namespace dx12 {
            static bool check_tearing_support() {
                bool allow_tearing = false;
                ComPtr<IDXGIFactory4> factory_4;
                if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory_4)))) {
                    ComPtr<IDXGIFactory5> factory_5;
                    if (SUCCEEDED(factory_4.As(&factory_5))) {
                        BOOL win32_allow_tearing = false;
                        if (FAILED(factory_5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &win32_allow_tearing, sizeof(BOOL)))) {
                            win32_allow_tearing = false;
                        }
                        allow_tearing = win32_allow_tearing;
                    }
                }
                return allow_tearing;
            }
            dx12_context::dx12_context(ref<dx12_object_factory> factory) {
                this->m_factory = factory;
                util::zero(this->m_fence_values.data(), this->m_fence_values.size() * sizeof(uint64_t));
            }
            void dx12_context::clear(glm::vec4 clear_color) {
                auto backbuffer = this->m_backbuffers[this->m_current_backbuffer];
                this->m_current_command_list = this->m_command_queues[D3D12_COMMAND_LIST_TYPE_DIRECT]->get_command_list();
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    backbuffer.Get(),
                    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
                this->m_current_command_list->ResourceBarrier(1, &barrier);
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(this->m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
                    this->m_current_backbuffer, this->m_rtv_descriptor_size);
                this->m_current_command_list->ClearRenderTargetView(rtv, &clear_color.x, 0, nullptr);
            }
            void dx12_context::make_current() {
                this->m_factory->m_current_context = this;
            }
            void dx12_context::draw_indexed(size_t index_count) {
                // todo: draw bound pipeline
            }
            void dx12_context::swap_buffers() {
                auto backbuffer = this->m_backbuffers[this->m_current_backbuffer];
                CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                    backbuffer.Get(),
                    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
                this->m_current_command_list->ResourceBarrier(1, &barrier);
                auto command_queue = this->m_command_queues[D3D12_COMMAND_LIST_TYPE_DIRECT];
                this->m_fence_values[this->m_current_backbuffer] = command_queue->execute_command_list(this->m_current_command_list);
                this->m_current_command_list.Reset();
                constexpr bool vsync = true;
                uint32_t present_flags = 0;
                uint32_t sync_interval = 0;
                if (vsync) {
                    sync_interval = 1;
                } else if (check_tearing_support()) {
                    present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                }
                check_hresult(this->m_swapchain->Present(sync_interval, present_flags), "[dx12 context] could not present");
                this->m_current_backbuffer = this->m_swapchain->GetCurrentBackBufferIndex();
                command_queue->wait_for_fence_value(this->m_fence_values[this->m_current_backbuffer]);
            }
            void dx12_context::setup_glfw() {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            }
            void dx12_context::setup_context() {
                this->enable_debug_layer();
                this->create_device();
                std::vector<D3D12_COMMAND_LIST_TYPE> command_queue_types = {
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    D3D12_COMMAND_LIST_TYPE_COPY,
                    D3D12_COMMAND_LIST_TYPE_COMPUTE
                };
                for (auto type : command_queue_types) {
                    auto queue = ref<dx12_command_queue>::create(this->m_device, type);
                    this->m_command_queues.insert({ type, queue });
                }
                int32_t width, height;
                glfwGetFramebufferSize(this->m_window, &width, &height);
                this->create_swapchain(glm::ivec2(width, height));
                this->m_current_backbuffer = this->m_swapchain->GetCurrentBackBufferIndex();
                this->m_rtv_descriptor_heap = this->create_descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, backbuffer_count);
                this->m_rtv_descriptor_size = this->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                this->m_srv_descriptor_heap = this->create_descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
                this->update_render_target_views();
            }
            void dx12_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                glm::ivec2 window_size = util::get_new_window_size(this->m_window);
                for (auto [type, queue] : this->m_command_queues) {
                    queue->flush();
                }
                for (uint32_t i = 0; i < backbuffer_count; i++) {
                    this->m_backbuffers[i].Reset();
                    this->m_fence_values[i] = this->m_fence_values[this->m_current_backbuffer];
                }
                DXGI_SWAP_CHAIN_DESC desc;
                util::zero(desc);
                check_hresult(this->m_swapchain->GetDesc(&desc), "[dx12 context] could not get swapchain description for resizing");
                check_hresult(this->m_swapchain->ResizeBuffers(backbuffer_count, window_size.x, window_size.y, desc.BufferDesc.Format, desc.Flags),
                    "[dx12 context] could not resize swapchain");
                this->m_current_backbuffer = this->m_swapchain->GetCurrentBackBufferIndex();
                this->update_render_target_views();
            }
            void dx12_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForOther(this->m_window, true);
                ImGui_ImplDX12_Init(this->m_device.Get(),
                    (int32_t)backbuffer_count,
                    DXGI_FORMAT_R8G8B8A8_UNORM, this->m_srv_descriptor_heap.Get(),
                    this->m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
                    this->m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart());
            }
            void dx12_context::shutdown_imgui_backends() {
                ImGui_ImplDX12_Shutdown();
                ImGui_ImplGlfw_Shutdown();
            }
            void dx12_context::call_imgui_backend_newframe() {
                ImGui_ImplDX12_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            }
            void dx12_context::render_imgui_draw_data(ImDrawData* data) {
                ImGui_ImplDX12_RenderDrawData(data, this->m_current_command_list.Get());
            }
            void dx12_context::update_render_target_views() {
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(this->m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
                for (uint32_t i = 0; i < backbuffer_count; i++) {
                    ComPtr<ID3D12Resource> backbuffer;
                    check_hresult(this->m_swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)), "[dx12 context] could not get backbuffer " + std::to_string(i));
                    this->m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, rtv_handle);
                    this->m_backbuffers[i] = backbuffer;
                    rtv_handle.Offset(this->m_rtv_descriptor_size);
                }
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
            void dx12_context::create_swapchain(glm::ivec2 size) {
                ComPtr<IDXGIFactory4> factory;
                create_dxgi_factory(factory);
                DXGI_SWAP_CHAIN_DESC1 desc;
                util::zero(desc);
                desc.Width = (uint32_t)size.x;
                desc.Height = (uint32_t)size.y;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.Stereo = false;
                desc.SampleDesc = { 1, 0 };
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.BufferCount = backbuffer_count;
                desc.Scaling = DXGI_SCALING_STRETCH;
                desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                if (check_tearing_support()) {
                    desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
                }
                ComPtr<IDXGISwapChain1> swapchain_1;
                HWND hwnd = glfwGetWin32Window(this->m_window);
                check_hresult(factory->CreateSwapChainForHwnd(
                    this->m_command_queues[D3D12_COMMAND_LIST_TYPE_DIRECT]->get_command_queue().Get(),
                    hwnd,
                    &desc,
                    nullptr,
                    nullptr,
                    &swapchain_1
                ), "[dx12 context] could not create swapchain");
                check_hresult(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER), "[dx12 context] could not make window association");
                check_hresult(swapchain_1.As(&this->m_swapchain), "[dx12 context] could not cast IDXGISwapChain1 to IDXGISwapChain4");
            }
            ComPtr<ID3D12DescriptorHeap> dx12_context::create_descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptor_count) {
                D3D12_DESCRIPTOR_HEAP_DESC desc;
                util::zero(desc);
                desc.NumDescriptors = descriptor_count;
                desc.Type = type;
                ComPtr<ID3D12DescriptorHeap> descriptor_heap;
                check_hresult(this->m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap)), "[dx12 context] could not create descriptor heap");
                return descriptor_heap;
            }
        }
    }
}