#pragma once

#include "vlpch.h"

#include "Violet/Core.h"
#include "Violet//Events/Event.h"

namespace Violet
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "Violet Engine",
					unsigned int width = 1200,
					unsigned int height = 800)
			:Title(title), Width(width), Height(height) {}
	};

	// Interface representing a desktop system based Window
	class VIOLET_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());

		virtual void* GetNativeWindow() const = 0;
	};

}