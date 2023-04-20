#pragma once

/*
*	- Wrapper class for ID3D12CommandQueue 
*/

#include <d3d12.h> //For D3D12 classes (ID3D12CommandQueue included)
#include <wrl.h>   //For Microsoft:WRL::ComPtr

#include <cstdint> //For uint_t
#include <queue>   //For std::queue

class CommandQueue
{
public:
	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
	virtual ~CommandQueue();

	//Get an available command list 
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	//Execute the command list and returns the fence value to wait for this command list to finish
	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint32_t Signal();
	bool IsFenceComplete(uint64_t fenceVale);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>     CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	struct CommandAllocatorEntry
	{
		//We will use this to track each allocator on the CPU side. This is the value that the specified allocator has to reach on the GPU. 
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	using CommandListQueue      = std::queue <Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	D3D12_COMMAND_LIST_TYPE m_CommandListType;

	Microsoft::WRL::ComPtr<ID3D12Device2>       m_D3D12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>  m_D3D12CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>         m_D3D12Fence;

	//the fence is continuous and it will be updated once an allocator has reached its end. 
	//So, the first allocator has CPU Side value as 1 and the fence is on 0. After this allocator ends, it will increment the fence to 1
	//Then, the fence being equal to the CPU fence value, it assumes that it can reuse this allocator. 
	HANDLE                  m_FenceEvent;

	//This fence value will be used as the main fence value. This is, we will increment this value and then pass it to the next allocator as being the CPU fence value.
	uint64_t                m_FenceValue;

	CommandAllocatorQueue   m_CommandAllocatorQueue;
	CommandListQueue        m_CommandListQueue;
};