#pragma once

/*
 * @brief This class is the abstract base class for DirectX 12 demos 
 */

#include <OS/Events/Events.h>

#include <memory> // for std::enable_shared_from_this
#include <string> // for std::wstring

class Window;

class Sandbox : public std::enable_shared_from_this<Sandbox>
{
public:
	//Create the demo using the specified window dimensions and if we will be using vsync.
	Sandbox(const std::wstring& name, int width, int height, bool vSync);
	virtual ~Sandbox();

	//Init the graphics API.
	virtual bool Initialize();

	//Load content required for the demo.
	virtual bool LoadContent() = 0;

	//Unload the demo content that was loaded by LoadContent.
	virtual void UnloadContent() = 0;

	//Destroy any resource used by the sandbox
	virtual void Destroy();

protected:
	friend class Window;

	//Update the game logic / tick function
	virtual void OnUpate(const UpdateEventArgs& e);

	//Render stuff
	virtual void OnRender(const RenderEventArgs& e);

	//Key events (released, pressed, moved etc) and destroy event.
	virtual void OnKeyPressed(const KeyEventArgs& e);

	virtual void OnKeyReleased(const KeyEventArgs& e);

	virtual void OnMouseMoved(const MouseMotionEventArgs& e);

	virtual void OnMouseButtonPressed(const MouseButtonEventArgs& e);

	virtual void OnMouseButtonReleased(const MouseButtonEventArgs& e);

	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	virtual void OnResize(ResizeEventArgs& e);

	virtual void OnWindowDestroy();

	std::shared_ptr<Window> m_pWindow;


private:
	std::wstring m_Name;
	int m_Width;
	int m_Height;
	bool m_vSync;
};
