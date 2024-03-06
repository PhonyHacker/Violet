#include <Violet.h>

// #include "imgui/imgui.h"

class ExampleLayer : public Violet::Layer {
public:
	ExampleLayer()
		:Layer("Example Layer") {

	}
	void OnUpdate() override {
		// VL_INFO("Example Layer Update");
		if (Violet::Input::IsKeyPressed(VL_KEY_TAB))
			VL_TRACE("Tav key is pressed (poll)!");
	}


	void OnEvent(Violet::Event& event) override {
		// VL_TRACE("{0}", event);
		/*
		if (event.GetEventType() == Violet::EventType::KeyPressed) {
			Violet::KeyPressedEvent& e = (Violet::KeyPressedEvent&)event;
			if (e.GetKeyCode() == VL_KEY_TAB)
				VL_TRACE("Tab key is pressed (event)!");
			VL_TRACE("{0}", (char)e.GetKeyCode());
		}
		*/
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