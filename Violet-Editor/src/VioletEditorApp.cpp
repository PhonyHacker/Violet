#include <Violet.h>
#include <Violet/Core/EntryPoint.h>

#include "EditorLayer.h"
namespace Violet {
	class VioletEditor : public Application
	{
	public:
		VioletEditor(const ApplicationSpecification& spec)
			:Application(spec)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "VioletEditor";
		spec.CommandLineArgs = args;

		return new VioletEditor(spec);
	}

}
