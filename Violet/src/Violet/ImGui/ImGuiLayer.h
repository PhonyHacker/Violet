#pragma once

#include "Violet/Core/Layer.h"

#include "Violet/Events/ApplicationEvent.h"
#include "Violet/Events/KeyEvent.h"
#include "Violet/Events/MouseEvent.h"

namespace Violet {
	class VIOLET_API ImGuiLayer : public Layer{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:				    
		float m_Time = 0.0f;
	};
}