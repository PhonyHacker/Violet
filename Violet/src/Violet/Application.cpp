#include "vlpch.h"

#include "Application.h"

#include "Violet/Events/ApplicationEvent.h"
#include "Violet/Log.h"

namespace Violet {
	Application::Application() {

	}

	Application::~Application() {

	}
	void  Application::Run() {
		WindowResizeEvent e(1200, 720);
		VL_TRACE(e);
		while (true);
	}
}
