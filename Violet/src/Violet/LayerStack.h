#pragma once

#include "Violet/Core.h"
#include "Layer.h"

namespace Violet {
	class VIOLET_API LayerStack {
	public:
		LayerStack();
		~LayerStack();

		// �㼶��ջ����㼶ʱ���������٣�ֻ�Ƴ���ջ
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}