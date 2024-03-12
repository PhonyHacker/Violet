#pragma once

#include <glm/glm.hpp>

namespace Violet {
	class  OrthographicCamera{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		inline const glm::vec3 GetPosition() { return m_Position; }
		void SetPosition(const glm::vec3 position) { m_Position = position; RecalculateViewMartix(); }
		void SetProjection(float left, float right, float bottom, float top);

		inline const float GetRotation() { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMartix(); }

		const glm::mat4 GetProjectionMartix()	  const { return m_ProjectionMatrix; }
		const glm::mat4 GetViewMartix()			  const { return m_ViewMatrix; }
		const glm::mat4 GetViewProjectionMartix() const { return m_ViewProjectionMatrix; }
	
	private:
		void RecalculateViewMartix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		float m_Rotation;
	};
}