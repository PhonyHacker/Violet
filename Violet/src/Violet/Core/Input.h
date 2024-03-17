#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Core/KeyCode.h"
#include "Violet/Core/MouseButtonCode.h"
namespace Violet {

	class VIOLET_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsMouseButtonPressed(MouseButtonCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};

}