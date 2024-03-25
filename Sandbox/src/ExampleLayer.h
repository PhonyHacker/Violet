#pragma once

#include "Violet.h"

class ExampleLayer : public Violet::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Violet::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Violet::Event& e) override;
private:
	Violet::ShaderLibrary m_ShaderLibrary;
	Violet::Ref<Violet::Shader> m_Shader;
	Violet::Ref<Violet::VertexArray> m_VertexArray;
	
	Violet::Ref<Violet::Shader> m_FlatColorShader;
	Violet::Ref<Violet::VertexArray> m_SquareVA;
	
	Violet::Ref<Violet::Texture2D> m_Texture, m_ChernoLogoTexture;

	Violet::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};
