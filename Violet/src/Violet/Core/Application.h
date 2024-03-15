#pragma once

#include "Violet/Core/Core.h"
#include "Violet/Events/Event.h"
#include "Window.h"
#include "Violet/Core/Layer.h"
#include "Violet/Core/LayerStack.h"
#include "Violet/Events/ApplicationEvent.h"

#include "Violet/ImGui/ImGuiLayer.h"

#include "Violet/Core/Timestep.h"

namespace Violet {
	class VIOLET_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }

		inline Timestep GetTimeSetp() { return m_LastFrameTime; };
	private:
		Timestep m_LastFrameTime = 0.0f;

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;
		
		ImGuiLayer* m_ImGuiLayer;

		LayerStack m_LayerStack;

		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}
