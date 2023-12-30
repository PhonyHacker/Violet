#pragma once

#include "Core.h"

namespace Violet {
	class VIOLET_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}
