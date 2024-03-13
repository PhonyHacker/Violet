#include "vlpch.h"

#include "Application.h"

#include "Violet/Renderer/Renderer.h"

#include "Violet/Core/Log.h"
#include "Violet/Core/Input.h"

#include <GLFW/glfw3.h>


namespace Violet {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
		// :m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		VL_PROFILE_FUNCTION();

		VL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application() {
		VL_PROFILE_FUNCTION();
		Renderer::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		VL_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		// VL_CORE_INFO("{0}", e);
		// VL_CORE_TRACE("{0}", e);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(e);
			if (e.IsHandled())
				break;
		}
	}
	void  Application::Run() {
		VL_PROFILE_FUNCTION();

		while (m_Running)
		{	
			VL_PROFILE_SCOPE("RunLoop");
			float time = (float)glfwGetTime();

			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				VL_PROFILE_SCOPE("LayerStackOnUpdate");

				for (Layer* layer : m_LayerStack) {
					layer->OnUpdate(timestep);
				}
			}

			// imgui
			{
				VL_PROFILE_SCOPE("LayerStackOnImGui");

				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();

		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		VL_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
			m_Minimized = true;
		else 
			m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}

	void Application::PushLayer(Layer* layer) {
		VL_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer) {
		VL_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
}
