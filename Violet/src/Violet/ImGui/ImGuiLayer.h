#pragma once

#include "Violet/Core/Layer.h"

#include "Violet/Events/ApplicationEvent.h"
#include "Violet/Events/KeyEvent.h"
#include "Violet/Events/MouseEvent.h"

namespace Violet {
	class ImGuiLayer : public Layer{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void SetDarkThemeColors();

		void Begin();
		void End();
		void BlockEvents(bool block) { m_BlockEvents = block; }

	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}