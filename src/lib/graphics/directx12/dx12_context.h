#pragma once
#include "dx12_object_factory.h"
namespace bve {
    namespace graphics {
        namespace dx12 {
            constexpr uint32_t backbuffer_count = 3;
            class dx12_context : public context {
            public:
                dx12_context(ref<dx12_object_factory> factory);
                virtual ~dx12_context() override;
                virtual void clear(glm::vec4 clear_color) override;
                virtual void make_current() override;
                virtual void draw_indexed(size_t index_count) override;
                ComPtr<ID3D12Device2> get_device() { return this->m_device; }
                ComPtr<ID3D12CommandQueue> get_command_queue() { return this->m_command_queue; }
            private:
                virtual void swap_buffers() override;
                virtual void setup_glfw() override;
                virtual void setup_context() override;
                virtual void resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) override;
                virtual void init_imgui_backends() override;
                virtual void shutdown_imgui_backends() override;
                virtual void call_imgui_backend_newframe() override;
                virtual void render_imgui_draw_data(ImDrawData* data) override;
                void update_render_target_views();
                void enable_debug_layer();
                void create_device();
                ComPtr<ID3D12CommandQueue> create_command_queue(D3D12_COMMAND_LIST_TYPE type);
                void create_swapchain(glm::ivec2 size);
                ComPtr<ID3D12DescriptorHeap> create_descriptor_heap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptor_count);
                ComPtr<ID3D12CommandAllocator> create_command_allocator(D3D12_COMMAND_LIST_TYPE type);
                ComPtr<ID3D12GraphicsCommandList> create_command_list(ComPtr<ID3D12CommandAllocator> allocator, D3D12_COMMAND_LIST_TYPE type);
                ComPtr<ID3D12Fence> create_fence();
                void* create_event_handle();
                uint64_t signal(ComPtr<ID3D12CommandQueue> queue, ComPtr<ID3D12Fence> fence, uint64_t& fence_value);
                void wait_for_fence_value(ComPtr<ID3D12Fence> fence, uint64_t fence_value, void* fence_event, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
                void flush(ComPtr<ID3D12CommandQueue> queue, ComPtr<ID3D12Fence> fence, void* fence_event, uint64_t& fence_value);
                ComPtr<ID3D12Device2> m_device;
                ComPtr<ID3D12CommandQueue> m_command_queue;
                ComPtr<IDXGISwapChain4> m_swapchain;
                uint32_t m_current_backbuffer;
                ComPtr<ID3D12DescriptorHeap> m_rtv_descriptor_heap, m_srv_descriptor_heap;
                uint32_t m_rtv_descriptor_size;
                std::array<ComPtr<ID3D12Resource>, backbuffer_count> m_backbuffers;
                std::array<ComPtr<ID3D12CommandAllocator>, backbuffer_count> m_command_allocators;
                ComPtr<ID3D12GraphicsCommandList> m_command_list;
                ComPtr<ID3D12Fence> m_fence;
                void* m_fence_event;
                uint64_t m_fence_value;
                std::array<uint64_t, backbuffer_count> m_frame_fence_values;
                ref<dx12_object_factory> m_factory;
            };
        }
    }
}