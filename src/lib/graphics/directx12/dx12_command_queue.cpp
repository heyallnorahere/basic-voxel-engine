#include "bve_pch.h"
#include "dx12_util.h"
#include "dx12_command_queue.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace dx12 {
            dx12_command_queue::dx12_command_queue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) {
                this->m_device = device;
                this->m_type = type;
                this->m_fence_value = 0;
                D3D12_COMMAND_QUEUE_DESC desc;
                util::zero(desc);
                desc.Type = this->m_type;
                desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                desc.NodeMask = 0;
                check_hresult(this->m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&this->m_command_queue)),
                    "[dx12 command queue] could not create command queue");
                check_hresult(this->m_device->CreateFence(this->m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->m_fence)),
                    "[dx12 command queue] could not create fence");
                this->m_fence_event = ::CreateEvent(nullptr, false, false, nullptr);
                if (!this->m_fence_event) {
                    throw std::runtime_error("[dx12 command queue] could not create fence event");
                }
            }
            dx12_command_queue::~dx12_command_queue() {
                this->flush();
                ::CloseHandle(this->m_fence_event);
            }
            ComPtr<ID3D12GraphicsCommandList2> dx12_command_queue::get_command_list() {
                ComPtr<ID3D12CommandAllocator> command_allocator;
                bool obtained_allocator = false;
                if (!this->m_command_allocator_queue.empty()) {
                    auto front = this->m_command_allocator_queue.front();
                    if (this->is_fence_complete(front.fence_value)) {
                        this->m_command_allocator_queue.pop();
                        command_allocator = front.command_allocator;
                        check_hresult(command_allocator->Reset(), "[dx12 command queue] could not reset command allocator");
                        obtained_allocator = true;
                    }
                }
                if (!obtained_allocator) {
                    command_allocator = this->create_command_allocator();
                }
                ComPtr<ID3D12GraphicsCommandList2> command_list;
                if (!this->m_command_list_queue.empty()) {
                    command_list = this->m_command_list_queue.front();
                    this->m_command_list_queue.pop();
                    check_hresult(command_list->Reset(command_allocator.Get(), nullptr), "[dx12 command queue] could not reset command list");
                } else {
                    command_list = this->create_command_list(command_allocator);
                }
                check_hresult(command_list->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), command_allocator.Get()),
                    "[dx12 command queue] could not assign command allocator to command list");
                return command_list;
            }
            uint64_t dx12_command_queue::execute_command_list(ComPtr<ID3D12GraphicsCommandList2> command_list) {
                check_hresult(command_list->Close(), "[dx12 command queue] could not close command list");
                ID3D12CommandAllocator* command_allocator;
                uint32_t data_size = (uint32_t)sizeof(size_t);
                check_hresult(command_list->GetPrivateData(__uuidof(ID3D12CommandAllocator), &data_size, &command_allocator),
                    "[dx12 command queue] could not get command allocator from command list");
                ID3D12CommandList* cmdlist = command_list.Get();
                this->m_command_queue->ExecuteCommandLists(1, &cmdlist);
                uint32_t fence_value = this->signal();
                this->m_command_allocator_queue.push({ fence_value, command_allocator });
                this->m_command_list_queue.push(command_list);
                command_allocator->Release();
                return fence_value;
            }
            uint64_t dx12_command_queue::signal() {
                uint64_t fence_value = ++this->m_fence_value;
                check_hresult(this->m_command_queue->Signal(this->m_fence.Get(), fence_value),
                    "[dx12 command queue] could not signal fence");
                return fence_value;
            }
            bool dx12_command_queue::is_fence_complete(uint64_t fence_value) {
                return this->m_fence->GetCompletedValue() >= fence_value;
            }
            void dx12_command_queue::wait_for_fence_value(uint64_t fence_value) {
                if (!this->is_fence_complete(fence_value)) {
                    this->m_fence->SetEventOnCompletion(fence_value, this->m_fence_event);
                    ::WaitForSingleObject(this->m_fence_event, std::numeric_limits<DWORD>::max());
                }
            }
            void dx12_command_queue::flush() {
                this->wait_for_fence_value(this->signal());
            }
            ComPtr<ID3D12CommandAllocator> dx12_command_queue::create_command_allocator() {
                ComPtr<ID3D12CommandAllocator> command_allocator;
                check_hresult(this->m_device->CreateCommandAllocator(this->m_type, IID_PPV_ARGS(&command_allocator)),
                    "[dx12 command queue] could not create command allocator");
                return command_allocator;
            }
            ComPtr<ID3D12GraphicsCommandList2> dx12_command_queue::create_command_list(ComPtr<ID3D12CommandAllocator> allocator) {
                ComPtr<ID3D12GraphicsCommandList2> command_list;
                check_hresult(this->m_device->CreateCommandList(0, this->m_type, allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)),
                    "[dx12 command queue] could not create command list");
                return command_list;
            }
        }
    }
}