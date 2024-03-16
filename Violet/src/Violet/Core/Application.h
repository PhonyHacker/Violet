#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Events/Event.h"
#include "Window.h"
#include "Violet/Core/Layer.h"
#include "Violet/Core/LayerStack.h"
#include "Violet/Events/ApplicationEvent.h"

#include "Violet/ImGui/ImGuiLayer.h"

#include "Violet/Core/Timestep.h"

namespace Violet {
	class Application
	{
	public:
		Application(const std::string& name = "Violet App");
		virtual ~Application();

		void Run();
		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

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
