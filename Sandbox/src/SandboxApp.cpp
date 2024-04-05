#include <Violet.h>
//#include <Violet/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public Violet::Application {
public:
	Sandbox(const Violet::ApplicationSpecification& specification)
		: Violet::Application(specification)
	{
		// PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
	~Sandbox()
	{
	
	}
};

Violet::Application* Violet::CreateApplication(Violet::ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Sandbox";
	spec.WorkingDirectory = "../Hazelnut";
	spec.CommandLineArgs = args;

	return new Sandbox(spec);
}
