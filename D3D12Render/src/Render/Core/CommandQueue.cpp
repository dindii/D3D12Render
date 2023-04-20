#include "CommandQueue.h"

#include <Utils/D3D12Check.h>
#include <Utils/BasicAssert.h>

CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) :
	m_FenceValue(0), m_CommandListType(type), m_D3D12Device(device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type     = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	Check(m_D3D12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_D3D12CommandQueue)));
	Check(m_D3D12Device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_D3D12Fence)));

	m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	D3D_ASSERT(m_FenceEvent, "Failed to create Windows Event (Fence)!");
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Check(m_D3D12Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
	Check(m_D3D12Device->CreateCommandList(0, m_CommandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
}

CommandQueue::~CommandQueue()
{

}

uint64_t CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{

}

uint32_t CommandQueue::Signal()
{

}

bool CommandQueue::IsFenceComplete(uint64_t fenceVale)
{

}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{

}

void CommandQueue::Flush()
{

}