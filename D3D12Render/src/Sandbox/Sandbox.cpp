#pragma once
#include "Sandbox.h"

#include <d3d12.h> //For D3D12 classes (ID3D12CommandQueue included)
#include <wrl.h>   //For Microsoft:WRL::ComPtr
#include <DirectXMath.h>

Sandbox::Sandbox(const std::wstring& name, int width, int height, bool vSync)
{

}

bool Sandbox::Initialize()
{
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxA(NULL, "Failed to verify DirectX Math Library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
	m_pWindow->RegisterCallbacks(shared_from_this());
	m_pWindow->Show();

	return true;
}

void Sandbox::Destroy()
{
	Application::Get().DestroyWindow(m_pWindow);
	m_pWindow.reset();
}
