#pragma once

#include "Violet.h"

class EditorLayer: public Violet::Layer {
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Violet::Timestep step) override;
	virtual void OnImGuiRender() override;

	virtual void OnEvent(Violet::Event& event) override;
protected:
	Violet::OrthographicCameraController m_CameraController;

	//Temp
	Violet::Ref<Violet::VertexArray> m_SquareVA;
	Violet::Ref<Violet::Shader> m_FlatColorShader;

	Violet::Ref<Violet::Framebuffer> m_Framebuffer;

	Violet::Ref<Violet::Texture2D> m_CheckerboardTexture;
	
	bool m_ViewportFocused = false, m_ViewportHovered = false;

	glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

	/*
	Violet::Ref<Violet::Texture2D> m_TextureAltas;
	Violet::Ref<Violet::SubTexture2D> m_TextureStair, m_TextureTree, m_TextureBush;

	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, Violet::Ref<Violet::SubTexture2D>> s_TextureMap;
	*/
private:
	Violet::Timestep m_Timestep = 0.0f;
};