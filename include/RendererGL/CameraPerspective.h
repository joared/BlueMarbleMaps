#pragma once
#include "Camera.h"

struct PerspectiveCamerInformation : CameraInformation
{
	float m_fov = 45.0f;
	float m_aspectRatio = 1.0f;
};

class CameraPerspective : public Camera
{
	PerspectiveCamerInformation m_cameraInfo;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

public:
	CameraPerspective(PerspectiveCamerInformation cameraInfo);
	~CameraPerspective();
	//Operations

	void pan(float x, float y, float z) override;
	void zoom(float zoomFactor) override;
	void roll(float rotation) override;
	void pitch(float rotation) override;
	void yaw(float rotation) override;

	void orbit(float xRot, float yRot);

	glm::mat4 calculateTranslations() override;

	PerspectiveCamerInformation getCameraInfo();
	void setCameraInfo(PerspectiveCamerInformation);

	//not allowed to set view matrix, must be calculated

	glm::mat4 getViewMatrix() override;
};