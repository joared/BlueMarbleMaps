#pragma once
#include "Camera.h"
#include "glm.hpp"

struct OrthographicCameraInformation : public CameraInformation
{
	float m_width = 1000.0f;
	float m_height = 1000.0f;
};

class CameraOrthographic : public Camera
{
	OrthographicCameraInformation m_cameraInfo;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

public:
	CameraOrthographic(OrthographicCameraInformation cameraInfo);
	~CameraOrthographic();
	//Operations

	void pan(float x, float y, float z) override;
	void zoom(float zoomFactor) override;
	void roll(float rotation) override;
	void pitch(float rotation) override;
	void yaw(float rotation) override;
	void orbit(float xRot, float yRot) override;

	glm::mat4 calculateTranslations() override;

	OrthographicCameraInformation getCameraInfo();
	void setCameraInfo(OrthographicCameraInformation);

	//not allowed to set view matrix, must be calculated

	glm::mat4 getViewMatrix() override;
};