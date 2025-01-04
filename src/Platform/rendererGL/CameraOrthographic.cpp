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
	m_cameraInfo.m_pos += x * m_cameraInfo.m_right;
	m_cameraInfo.m_pivot += x * m_cameraInfo.m_right;

	m_cameraInfo.m_pos += y * m_cameraInfo.m_up;
	m_cameraInfo.m_pivot += y * m_cameraInfo.m_up;

	m_cameraInfo.m_pos += z * -m_cameraInfo.m_cameraFace;
	m_cameraInfo.m_pivot += z * -m_cameraInfo.m_cameraFace;
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
	float rot = rotation * m_cameraInfo.m_sensitivity;
	m_cameraInfo.m_roll += rot;
	if (m_cameraInfo.m_roll >= 360.0f)
		m_cameraInfo.m_roll -= 360.0f;
	else if (m_cameraInfo.m_roll <= -360.0f)
		m_cameraInfo.m_roll += 360.0f;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rot), m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_up = glm::mat3(rotationMatrix) * m_cameraInfo.m_up;
	m_cameraInfo.m_right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), -m_cameraInfo.m_cameraFace));
}

void CameraOrthographic::pitch(float rotation)
{
	if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_right);
	m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_up = glm::cross(-m_cameraInfo.m_cameraFace, m_cameraInfo.m_right);
}
void CameraOrthographic::yaw(float rotation)
{
	if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_up);
	m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), -m_cameraInfo.m_cameraFace));
}

void CameraOrthographic::orbit(float xRot, float yRot)
{
	if (m_cameraInfo.m_pos == m_cameraInfo.m_pivot) return;
	glm::mat4 xRotMatrix = glm::mat4(1.0f);
	glm::mat4 yRotMatrix = glm::mat4(1.0f);
	glm::vec4 tempPos = glm::vec4(m_cameraInfo.m_pos, 1.0f);
	if (xRot != 0)
	{
		xRotMatrix = glm::rotate(xRotMatrix, xRot, glm::vec3(0.0f, 1.0f, 0.0f));
		tempPos = xRotMatrix * (tempPos - glm::vec4(m_cameraInfo.m_pivot, 1.0f)) + glm::vec4(m_cameraInfo.m_pivot, 1.0f);
	}
	if (yRot != 0)
	{
		yRotMatrix = glm::rotate(yRotMatrix, yRot, m_cameraInfo.m_right);
		tempPos = yRotMatrix * (tempPos - glm::vec4(m_cameraInfo.m_pivot, 1.0f)) + glm::vec4(m_cameraInfo.m_pivot, 1.0f);;
	}
	float diff = glm::length(glm::vec3(tempPos) - m_cameraInfo.m_pivot);
	m_cameraInfo.m_pos = glm::vec3(tempPos);
	if (xRot != 0 || yRot != 0)
	{
		m_cameraInfo.m_cameraFace = glm::normalize(m_cameraInfo.m_pivot - m_cameraInfo.m_pos);
		m_cameraInfo.m_right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), -m_cameraInfo.m_cameraFace));
		m_cameraInfo.m_up = glm::cross(-m_cameraInfo.m_cameraFace, m_cameraInfo.m_right);
	}
}

glm::mat4 CameraOrthographic::calculateTranslations()
{
	m_projMatrix = glm::ortho(m_cameraInfo.m_width, 0.0f, 0.0f, m_cameraInfo.m_height, m_cameraInfo.m_near, m_cameraInfo.m_far);
	m_viewMatrix = m_projMatrix * glm::lookAt(m_cameraInfo.m_pos, m_cameraInfo.m_cameraFace + m_cameraInfo.m_pivot, m_cameraInfo.m_up);
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