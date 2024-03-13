#include "vlpch.h"
#include "OpenGLVertexArray.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Violet {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Violet::ShaderDataType::Float:    return GL_FLOAT;
		case Violet::ShaderDataType::Float2:   return GL_FLOAT;
		case Violet::ShaderDataType::Float3:   return GL_FLOAT;
		case Violet::ShaderDataType::Float4:   return GL_FLOAT;
		case Violet::ShaderDataType::Mat3:     return GL_FLOAT;
		case Violet::ShaderDataType::Mat4:     return GL_FLOAT;
		case Violet::ShaderDataType::Int:      return GL_INT;
		case Violet::ShaderDataType::Int2:     return GL_INT;
		case Violet::ShaderDataType::Int3:     return GL_INT;
		case Violet::ShaderDataType::Int4:     return GL_INT;
		case Violet::ShaderDataType::Bool:     return GL_BOOL;
		}

		VL_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		VL_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		VL_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		VL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		VL_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		VL_PROFILE_FUNCTION();

		VL_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		VL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

	const std::vector<std::shared_ptr<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffer() const
	{
		return m_VertexBuffers;
	}

}