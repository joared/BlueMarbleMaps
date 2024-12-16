#pragma once
#include "glm.hpp"
class Camera2D
{
	glm::vec3 m_pos;
	glm::vec3 m_right;
	glm::vec3 m_up;
	glm::vec3 m_cameraFace;

	float m_zoom;
	float m_roll;
	float m_sensitivity;

	glm::mat4 m_viewMatrix;

public:
	Camera2D(glm::vec3 up, glm::vec3 right);
	~Camera2D();
	//Operations

	void pan(int x, int y);
	void zoom(float zoomFactor);
	void roll(float rotation);

	void calculateTranslations();
	//Getters and setters directly couple to the data members

	glm::vec3 getPos();
	void setPos(glm::vec3 pos);

	float getSensitivity();
	void setSensitivity(float sensitivity);

	float getRoll();
	void setRoll(float roll);
	//not allowed to set view matrix, must be calculated

	glm::mat4 getViewMatrix();
};