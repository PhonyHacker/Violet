#pragma once

#ifdef VL_PLATFORM_WINDOWS

extern Violet::Application* Violet::CreateApplication();

int main(int argc, char** argv)
{
	printf("Violet Engine\n");
	auto app = Violet::CreateApplication();
	app->Run();
	delete app;
}

#endif