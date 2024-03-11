#include <Violet.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

class ExampleLayer : public Violet::Layer {
public:
	ExampleLayer()
		:Layer("Example Layer") ,m_Camera(-1.6f, 1.6f, -0.9f, 0.9f){
		// vertex Array
		// vertex Buffer
		// Index Buffer
		// ~Shader

		// glGenVertexArrays(1, &m_VertexArray);
		// glBindVertexArray(m_VertexArray);
		m_VertexArray.reset(Violet::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		Violet::Ref<Violet::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Violet::VertexBuffer::Create(vertices, sizeof(vertices)));
		Violet::BufferLayout layout = {
			{ Violet::ShaderDataType::Float3, "a_Position" },
			{ Violet::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		Violet::Ref<Violet::IndexBuffer> indexBuffer;
		indexBuffer.reset(Violet::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Violet::VertexArray::Create());
		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Violet::Ref<Violet::VertexBuffer> squareVB;
		squareVB.reset(Violet::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Violet::ShaderDataType::Float3, "a_Position" },
			{ Violet::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Violet::Ref<Violet::IndexBuffer> squareIB;
		squareIB.reset(Violet::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main(){
				v_Position = a_Position * 0.5 + 0.5;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform *vec4(a_Position, 1.0);
				
			}

		)";
		std::string framentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main(){
				color = vec4(v_Position, 1.0);
				color = v_Color;
			}

		)";

		m_Shader.reset(Violet::Shader::Create(vertexSrc, framentSrc));

		std::string flarColorShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;			

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader.reset(Violet::Shader::Create(flarColorShaderVertexSrc, flatColorShaderFragmentSrc));

		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec2 v_TexCoord;
			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			
			uniform sampler2D u_Texture;
			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_TextureShader.reset(Violet::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));

		m_Texture = Violet::Texture2D::Create("assets/textures/test.png");

		std::dynamic_pointer_cast<Violet::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Violet::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Violet::Timestep timestep) override {
		// VL_INFO("Example Layer Update");
		if (Violet::Input::IsKeyPressed(VL_KEY_TAB))
			VL_TRACE("Tav key is pressed (poll)!");
		else if (Violet::Input::IsKeyPressed(VL_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * timestep;
		else if (Violet::Input::IsKeyPressed(VL_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * timestep;
		else if (Violet::Input::IsKeyPressed(VL_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * timestep;
		else if (Violet::Input::IsKeyPressed(VL_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * timestep;
		else if (Violet::Input::IsKeyPressed(VL_KEY_Q))
			m_CameraRotation += m_CameraRoatateSpeed * timestep;
		else if (Violet::Input::IsKeyPressed(VL_KEY_E))
			m_CameraRotation -= m_CameraRoatateSpeed * timestep;


		Violet::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Violet::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		// Render data loading

		Violet::Renderer::BeginScene(m_Camera);

		std::dynamic_pointer_cast<Violet::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Violet::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Violet::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}
		m_Texture->Bind();
		Violet::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		// Violet::Renderer::Submit(m_Shader, m_VertexArray, scale);
		
		Violet::Renderer::EndScene();
	}

	void OnEvent(Violet::Event& event) override {
		Violet::EventDispatcher dispatcher(event);
		// dispatcher.Dispatch<Violet::KeyPressedEvent>(VL_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));

	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	bool OnKeyPressedEvent(Violet::KeyPressedEvent& event) {
		if (event.GetKeyCode() == VL_KEY_LEFT)
			m_CameraPosition.x -= m_CameraMoveSpeed;
		if (event.GetKeyCode() == VL_KEY_RIGHT)
			m_CameraPosition.x += m_CameraMoveSpeed;
		if (event.GetKeyCode() == VL_KEY_UP)
			m_CameraPosition.y += m_CameraMoveSpeed;
		if (event.GetKeyCode() == VL_KEY_DOWN)
			m_CameraPosition.y -= m_CameraMoveSpeed;
		if (event.GetKeyCode() == VL_KEY_Q)
			m_CameraRotation += m_CameraRoatateSpeed;
		if (event.GetKeyCode() == VL_KEY_E)
			m_CameraRotation -= m_CameraRoatateSpeed;

		return false;
	}
private:
	Violet::Ref<Violet::Shader> m_Shader;
	Violet::Ref<Violet::VertexArray> m_VertexArray;

	Violet::Ref<Violet::Shader> m_FlatColorShader, m_TextureShader;
	Violet::Ref<Violet::VertexArray> m_SquareVA;

	Violet::Ref<Violet::Texture2D> m_Texture;
	Violet::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = {0, 0, 0};
	float m_CameraMoveSpeed = 1.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRoatateSpeed = 90.0f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};


class Sandbox : public Violet::Application {
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{
	
	}
};

Violet::Application* Violet:: CreateApplication() {
	return new Sandbox();
}