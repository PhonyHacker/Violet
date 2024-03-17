#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Core/KeyCode.h"
#include "Violet/Core/MouseButtonCode.h"

#include <glm/glm.hpp>
namespace Violet {

	class VIOLET_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsMouseButtonPressed(MouseButtonCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};

}