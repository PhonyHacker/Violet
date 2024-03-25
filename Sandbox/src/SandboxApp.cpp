#include <Violet.h>
//#include <Violet/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public Violet::Application {
public:
	Sandbox()
	{
		// PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
	~Sandbox()
	{
	
	}
};

Violet::Application* Violet:: CreateApplication() {
	return new Sandbox();
}

