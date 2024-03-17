#pragma once

#include "Violet.h"

namespace Violet {
	class EditorLayer : public Layer {
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep step) override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event& event) override;
	protected:
		OrthographicCameraController m_CameraController;

		//Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Texture2D> m_CheckerboardTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;

		/*
		Ref<Texture2D> m_TextureAltas;
		Ref<SubTexture2D> m_TextureStair, m_TextureTree, m_TextureBush;

		uint32_t m_MapWidth, m_MapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> s_TextureMap;
		*/
	private:
		Timestep m_Timestep = 0.0f;
	};
}