#pragma once
#include "glm.hpp"

struct CameraInformation
{
	glm::vec3 m_pos = glm::vec3(0.0f,0.0f,100.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_up = glm::vec3(0.0f,1.0f,0.0f);
	glm::vec3 m_cameraFace = glm::vec3(0.0f,0.0f,-1.0f);
	glm::vec3 m_pivot = glm::vec3(0.0f, 0.0f, 0.0f);

	float m_zoom = 1.0f;
	float m_roll = 0.0f;
	float m_sensitivity = 1.0f;
	float m_near = 0.1f;
	float m_far = 1000.0f;

	bool m_directional = true;
};

class Camera
{
public:
	virtual ~Camera() = default;
	virtual void pan(float x, float y, float z) = 0;
	virtual void zoom(float zoomFactor) = 0;
	virtual void roll(float rotation) = 0;
	virtual void pitch(float rotation) = 0;
	virtual void yaw(float rotation) = 0;
	virtual void orbit(float xRot, float yRot) = 0;

	virtual glm::mat4& calculateTranslations() = 0;

	virtual glm::mat4& getViewMatrix() = 0;
};