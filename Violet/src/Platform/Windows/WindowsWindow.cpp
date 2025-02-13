#include "vlpch.h"
#include "WindowsWindow.h"

#include "Violet/Events/ApplicationEvent.h"
#include "Violet/Events/MouseEvent.h"
#include "Violet/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include <stb_image.h>


namespace Violet {
	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		VL_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Scope<Window> Window::Create(const WindowProps& props)
	{
		#ifdef VL_PLATFORM_WINDOWS
				return CreateScope<WindowsWindow>(props);
		#else
				VL_CORE_ASSERT(false, "Unknown platform!");
				return nullptr;
		#endif
	}

	
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		VL_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow() 
	{
		VL_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		VL_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		VL_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);


		if (s_GLFWWindowCount == 0)
		{
			VL_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
			VL_CORE_ASSERT(success, "Could not intialize GLFW!");

			glfwSetErrorCallback(GLFWErrorCallback);
		}
		{
			VL_PROFILE_SCOPE("glfwCreateWindow");

			m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);

			#pragma region 设置GLFW窗口图标
			int width, height, channels;
			unsigned char* icon_pixels = stbi_load("assets/icons/icon.png", &width, &height, &channels, STBI_rgb_alpha);
			// 使用像素数据创建 GLFW 窗口图标
			GLFWimage icon;
			icon.pixels = icon_pixels;
			icon.width = width;
			icon.height = height;
			glfwSetWindowIcon(m_Window, 1, &icon);
			stbi_image_free(icon_pixels);
			#pragma endregion

			s_GLFWWindowCount++;
		}
		
		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();


		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// 设置GLFW回调
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				
			switch(action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double x0ffset, double y0ffset) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				
			MouseScrolledEvent event((float)x0ffset, (float)y0ffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);

		});
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				KeyTypedEvent event(keycode);
				data.EventCallback(event);
			});
	}

	void WindowsWindow::Shutdown()
	{
		VL_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		if (--s_GLFWWindowCount == 0) {
			VL_CORE_INFO("Terminating GLFW");
			glfwTerminate();
		}
	}

	void WindowsWindow::OnUpdate()
	{
		VL_PROFILE_FUNCTION();

		glfwPollEvents();

		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled) glfwSwapInterval(1);
		else glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}
