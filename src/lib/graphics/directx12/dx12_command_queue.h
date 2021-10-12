#pragma once
namespace bve {
    namespace graphics {
        namespace dx12 {
            class dx12_command_queue : public ref_counted {
            public:
                dx12_command_queue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
                ~dx12_command_queue();
                dx12_command_queue(const dx12_command_queue&) = delete;
                dx12_command_queue& operator=(const dx12_command_queue&) = delete;
                D3D12_COMMAND_LIST_TYPE get_type() { return this->m_type; }
                ComPtr<ID3D12CommandQueue> get_command_queue() { return this->m_command_queue; }
                ComPtr<ID3D12GraphicsCommandList2> get_command_list();
                uint64_t execute_command_list(ComPtr<ID3D12GraphicsCommandList2> command_list);
                uint64_t signal();
                bool is_fence_complete(uint64_t fence_value);
                void wait_for_fence_value(uint64_t fence_value);
                void flush();
            private:
                ComPtr<ID3D12CommandAllocator> create_command_allocator();
                ComPtr<ID3D12GraphicsCommandList2> create_command_list(ComPtr<ID3D12CommandAllocator> allocator);
                struct command_allocator_entry {
                    uint64_t fence_value;
                    ComPtr<ID3D12CommandAllocator> command_allocator;
                };
                D3D12_COMMAND_LIST_TYPE m_type;
                ComPtr<ID3D12Device2> m_device;
                ComPtr<ID3D12CommandQueue> m_command_queue;
                ComPtr<ID3D12Fence> m_fence;
                void* m_fence_event;
                uint64_t m_fence_value;
                std::queue<command_allocator_entry> m_command_allocator_queue;
                std::queue<ComPtr<ID3D12GraphicsCommandList2>> m_command_list_queue;
            };
        }
    }
}