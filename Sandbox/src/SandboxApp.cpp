#include <Violet.h>
class Sandbox : public Violet::Application {
public:
	Sandbox()
	{

	}
	~Sandbox()
	{
	
	}
};

Violet::Application* Violet:: CreateApplication() {
	return new Sandbox();
}