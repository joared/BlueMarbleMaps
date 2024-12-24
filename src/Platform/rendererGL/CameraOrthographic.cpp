#include "CameraOrthographic.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"


CameraOrthographic::CameraOrthographic(OrthographicCameraInformation cameraInfo)
   :m_cameraInfo(cameraInfo),
	m_viewMatrix(glm::mat4(1.0f)),
	m_projMatrix(glm::mat4(1.0f))
{

}
CameraOrthographic::~CameraOrthographic()
{
	
}

void CameraOrthographic::pan(float x, float y, float z)
{
	m_cameraInfo.m_pos += glm::vec3(x, y, z) * m_cameraInfo.m_sensitivity;
}
void CameraOrthographic::zoom(float zoomFactor)
{
	m_cameraInfo.m_zoom += zoomFactor;
	if (m_cameraInfo.m_width - zoomFactor < 10.0f
		|| m_cameraInfo.m_height - zoomFactor < 10.0f)
	{
		m_cameraInfo.m_width = 10.0f;
		m_cameraInfo.m_height = 10.0f;
	}
	else if (m_cameraInfo.m_width - zoomFactor > 10000.0f
		|| m_cameraInfo.m_height - zoomFactor > 10000.0f)
	{
		m_cameraInfo.m_width = 10000.0f;
		m_cameraInfo.m_height = 10000.0f;
	}
	else
	{
		m_cameraInfo.m_width -= zoomFactor;
		m_cameraInfo.m_height -= zoomFactor;
	}
}
void CameraOrthographic::roll(float rotation)
{
	m_cameraInfo.m_roll += rotation * m_cameraInfo.m_sensitivity;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraInfo.m_roll), m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_up = glm::mat3(rotationMatrix) * m_cameraInfo.m_up;
}

void CameraOrthographic::pitch(float rotation)
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_right);
	m_cameraInfo.m_cameraFace = glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace;
}
void CameraOrthographic::yaw(float rotation)
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_up);
	m_cameraInfo.m_cameraFace = glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace;
}

glm::mat4 CameraOrthographic::calculateTranslations()
{
	m_projMatrix = glm::ortho(m_cameraInfo.m_width, 0.0f, 0.0f, m_cameraInfo.m_height, -m_cameraInfo.m_near, -m_cameraInfo.m_far);
	if(m_cameraInfo.m_directional) 
		m_viewMatrix = m_projMatrix * glm::lookAt(m_cameraInfo.m_pos, m_cameraInfo.m_pos+ m_cameraInfo.m_cameraFace, m_cameraInfo.m_up);
	else
		m_viewMatrix = m_projMatrix * glm::lookAt(m_cameraInfo.m_pos, m_cameraInfo.m_cameraFace, m_cameraInfo.m_up);
	return m_viewMatrix;
}

glm::mat4 CameraOrthographic::getViewMatrix()
{
	return m_viewMatrix;
}

OrthographicCameraInformation CameraOrthographic::getCameraInfo()
{
	return m_cameraInfo;
}
void CameraOrthographic::setCameraInfo(OrthographicCameraInformation cameraInfo)
{
	m_cameraInfo = cameraInfo;
}