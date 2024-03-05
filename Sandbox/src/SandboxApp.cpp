#include <Violet.h>

class ExampleLayer : public Violet::Layer {
public:
	ExampleLayer()
		:Layer("Example Layer") {}
	void OnUpdate() override {
		VL_INFO("Example Layer Update");
	}
	void OnEvent(Violet::Event& event) override {
		VL_TRACE("{0}", event);
	}
};


class Sandbox : public Violet::Application {
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{
	
	}
};

Violet::Application* Violet:: CreateApplication() {
	return new Sandbox();
}