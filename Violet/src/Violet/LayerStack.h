#pragma once

#include "Violet/Core.h"
#include "Layer.h"

namespace Violet {
	class VIOLET_API LayerStack {
	public:
		LayerStack();
		~LayerStack();

		// 层级堆栈管理层级时不进行销毁，只移除堆栈
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;
	};
}