#pragma once

#ifdef VL_PLATFORM_WINDOWS

extern Violet::Application* Violet::CreateApplication();

int main(int argc, char** argv)
{
	Violet::Log::Init();
	VL_CORE_WARN("Initialized Log!");
	VL_INFO("Hello!");

	auto app = Violet::CreateApplication();
	app->Run();
	delete app;
}

#endif