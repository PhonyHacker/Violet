#include <Violet.h>
#include <Violet/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Violet {

	class VioletEditor : public Application
	{
	public:
		VioletEditor(ApplicationCommandLineArgs args)
			: Application("VioletEditor", args)
		{
			PushLayer(new EditorLayer());
		}

		~VioletEditor()
		{
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new VioletEditor(args);
	}

}