#include <Violet.h>
#include <Violet/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Violet {

	class HazelEditor : public Application
	{
	public:
		HazelEditor()
			: Application("Violet Editor")
		{
			PushLayer(new EditorLayer());
		}

		~HazelEditor()
		{
		}
	};

	Application* CreateApplication()
	{
		return new HazelEditor();
	}

}