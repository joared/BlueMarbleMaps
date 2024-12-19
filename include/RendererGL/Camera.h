#pragma once
#include "glm.hpp"
class Camera
{
	glm::vec3 m_pos;
	glm::vec3 m_right;
	glm::vec3 m_up;
	glm::vec3 m_cameraFace;

	float m_zoom;
	float m_roll;
	float m_sensitivity;

	float m_width;
	float m_height;
	float m_near;
	float m_far;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_proj;

	bool m_directional;

public:
	Camera(glm::vec3 position, glm::vec3 cameraDirection, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f), float sensitivity = 1.0f);
	~Camera();
	//Operations

	void pan(float x, float y, float z);
	void zoom(float zoomFactor);
	void roll(float rotation);

	glm::mat4 calculateTranslations();
	//Getters and setters directly couple to the data members

	void setProjectionOrtographic(float width, float height, float near, float far);
	void setProjectionPerspective(float fov, float aspectRatio, float near, float far);

	glm::vec3 getPos();
	void setPos(glm::vec3 pos);

	float getSensitivity();
	void setSensitivity(float sensitivity);

	float getRoll();
	void setRoll(float roll);

	glm::vec3 getCameraFace();
	void setCameraFace(glm::vec3 direction);

	bool isDirectional();
	void setDirectional(bool directional);
	//not allowed to set view matrix, must be calculated

	glm::mat4 getViewMatrix();
};