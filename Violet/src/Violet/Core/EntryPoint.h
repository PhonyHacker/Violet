#pragma once

#ifdef VL_PLATFORM_WINDOWS

extern Violet::Application* Violet::CreateApplication();

int main(int argc, char** argv)
{
	Violet::Log::Init();
	VL_PROFILE_BEGIN_SESSION("Startup", "VioletProfile-Startup.json");
	auto app = Violet::CreateApplication();
	VL_PROFILE_END_SESSION();

	VL_PROFILE_BEGIN_SESSION("Runtime", "VioletProfile-Runtime.json");
	app->Run();
	VL_PROFILE_END_SESSION();

	VL_PROFILE_BEGIN_SESSION("Shutdown", "VioletProfile-Shutdown.json");
	delete app;
	VL_PROFILE_END_SESSION();

}

#endif