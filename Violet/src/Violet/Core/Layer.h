#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Events/Event.h"
#include "Violet/Core/Timestep.h"

namespace Violet {
	class VIOLET_API Layer {
	public :
		Layer(const std::string& name = "Layer");
		virtual ~Layer();
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep step) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() {}

		inline const std::string& GetName() const {return m_DebugName;}
	protected:
		std::string m_DebugName;
	};

}