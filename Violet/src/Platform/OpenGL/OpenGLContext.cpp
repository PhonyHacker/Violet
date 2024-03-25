#include "vlpch.h"
#include "OpenGLContext.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Violet {
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		VL_CORE_ASSERT(windowHandle, "Window Handle is null!");
	}
	void OpenGLContext::Init() {
		VL_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);

		// GLAD¿â³õÊ¼»¯
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VL_CORE_ASSERT(status, "Faild to initialize Glad!");

		VL_CORE_INFO("OpenGL Info: ");
		VL_CORE_INFO(" Vendor: {0} ", glGetString(GL_VENDOR));
		VL_CORE_INFO(" Renderer : {0} ", glGetString(GL_RENDERER));
		VL_CORE_INFO(" Version: {0} ", glGetString(GL_VERSION));

		VL_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Violet requires at least OpenGL version 4.5!");
	}
	void OpenGLContext::SwapBuffers() 
	{
		VL_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}


}