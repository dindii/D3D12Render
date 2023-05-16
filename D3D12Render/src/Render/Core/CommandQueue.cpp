#include "CommandQueue.h"

#include <Utils/D3D12Check.h>
#include <Utils/BasicAssert.h>

using namespace Microsoft::WRL;

namespace Render
{
	CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type) :
		m_FenceValue(0), m_CommandListType(type), m_D3D12Device(device)
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
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

		return commandList;
	}

	CommandQueue::~CommandQueue()
	{

	}

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
	{
		return m_D3D12CommandQueue;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::GetCommandList()
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ComPtr<ID3D12GraphicsCommandList2> commandList;

		if (!m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
			m_CommandAllocatorQueue.pop();

			Check(commandAllocator->Reset());
		}
		else
		{
			commandAllocator = CreateCommandAllocator();
		}


		if (!m_CommandListQueue.empty())
		{
			commandList = m_CommandListQueue.front();
			m_CommandListQueue.pop();

			Check(commandList->Reset(commandAllocator.Get(), nullptr));
		}
		else
		{
			commandList = CreateCommandList(commandAllocator);
		}

		Check(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

		return commandList;
	}

	uint64_t CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
	{
		commandList->Close();

		ID3D12CommandAllocator* commandAllocator;
		uint32_t dataSize = sizeof(commandAllocator);
		Check(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

		ID3D12CommandList* const ppCommandLists[] = { commandList.Get() };

		m_D3D12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
		uint64_t fenceValue = Signal();

		m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
		m_CommandListQueue.push(commandList);


		//The ownership of this command allocator is now on the command allocator queue pointer,
		//so we can release this temporary ref here.
		commandAllocator->Release();

		return fenceValue;
	}

	uint64_t CommandQueue::Signal()
	{
		//Get the actual fence value and increment in the CPU
		uint64_t fenceValueForSignal = ++m_FenceValue;

		//Ask for the GPU to update its fence with this CPU value. (this is run on the GPU, the CPU will not stop here, we will continue through our function)
		Check(m_D3D12CommandQueue->Signal(m_D3D12Fence.Get(), fenceValueForSignal));

		//Returns the value that we want the GPU to be on. We then will use this value to compare if the GPU has reached our fence. This is, we will stall until this fence is
		//equal fenceValueForSignal.
		return fenceValueForSignal;
	}

	bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
	{
		if (m_D3D12Fence->GetCompletedValue() >= fenceValue)
		{
			return true;
		}

		return false;
	}

	void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
	{
		//We check if the GPU has updated our fence to the CPU fence value
		if (!IsFenceComplete(fenceValue))
		{
			//If not, we will ask this fence to trigger this event once it reached the desired fence (usually, the CPU fence value)
			Check(m_D3D12Fence->SetEventOnCompletion(fenceValue, m_FenceEvent));

			//And now, we will stall the CPU until this event is triggered (i.e: until the GPU finishes its work).
			//You can optionally set for how long you want to wait. In our case, we will wait for millions of years, or in this case, for a INFINITE time.
			::WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	void CommandQueue::Flush()
	{
		uint64_t fenceValueToWait = Signal();
		WaitForFenceValue(m_FenceValue);
	}
};