#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Core/Application.h"
#include "Violet/Debug/Instrumentor.h"

#ifdef VL_PLATFORM_WINDOWS

extern Violet::Application* Violet::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)	
{
	Violet::Log::Init();
 	VL_PROFILE_BEGIN_SESSION("Startup", "VioletProfile-Startup.json");
	auto app = Violet::CreateApplication({ argc, argv });
	VL_PROFILE_END_SESSION();

	VL_PROFILE_BEGIN_SESSION("Runtime", "VioletProfile-Runtime.json");
	app->Run();
	VL_PROFILE_END_SESSION();

	VL_PROFILE_BEGIN_SESSION("Shutdown", "VioletProfile-Shutdown.json");
	delete app;
	VL_PROFILE_END_SESSION();

}

#endif