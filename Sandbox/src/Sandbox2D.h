#pragma once

#include "Violet.h"
#include "ParticleSystem.h"

class Sandbox2D : public Violet::Layer {
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

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

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

	Violet::Ref<Violet::Texture2D> m_TextureAltas;
	Violet::Ref<Violet::SubTexture2D> m_TextureStair, m_TextureTree, m_TextureBush;

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;

	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, Violet::Ref<Violet::SubTexture2D>> s_TextureMap;
private:
	Violet::Timestep m_Tiemstep = 0.0f;
};