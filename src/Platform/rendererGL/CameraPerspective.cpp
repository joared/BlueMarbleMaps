#include "CameraPerspective.h"
#include "gtc/matrix_transform.hpp"

CameraPerspective::CameraPerspective(PerspectiveCamerInformation cameraInfo)
    :m_cameraInfo(cameraInfo),
	 m_viewMatrix(glm::mat4(1.0f)),
	 m_projMatrix(glm::mat4(1.0f))
{

}
CameraPerspective::~CameraPerspective()
{

}
//Operations

void CameraPerspective::pan(float x, float y, float z)
{
	m_cameraInfo.m_pos += x * m_cameraInfo.m_right;
	m_cameraInfo.m_pivot += x * m_cameraInfo.m_right;
	
	m_cameraInfo.m_pos += y * m_cameraInfo.m_up;
	m_cameraInfo.m_pivot += y * m_cameraInfo.m_up;

	m_cameraInfo.m_pos += z * -m_cameraInfo.m_cameraFace;
	m_cameraInfo.m_pivot += z * -m_cameraInfo.m_cameraFace;
}
void CameraPerspective::zoom(float zoomFactor)
{
	
	if (m_cameraInfo.m_fov - zoomFactor < 1.0f)
	{
		m_cameraInfo.m_fov = 1.0f;
		m_cameraInfo.m_zoom += zoomFactor;
	}
	else if (m_cameraInfo.m_fov - zoomFactor > 90.0f)
	{
		m_cameraInfo.m_fov = 90.0f;
		m_cameraInfo.m_zoom += zoomFactor;
	}
	else
	{
		m_cameraInfo.m_fov -= zoomFactor;
		m_cameraInfo.m_zoom += zoomFactor;
	}
}
void CameraPerspective::roll(float rotation)
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

void CameraPerspective::pitch(float rotation)
{
	if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_right);
	m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_up = glm::cross(-m_cameraInfo.m_cameraFace, m_cameraInfo.m_right);
}
void CameraPerspective::yaw(float rotation)
{
	if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_up);
	m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
	m_cameraInfo.m_right = glm::normalize(glm::cross(glm::vec3(0.0f,1.0f,0.0f), -m_cameraInfo.m_cameraFace));
}

void CameraPerspective::orbit(float xRot, float yRot)
{
	if (m_cameraInfo.m_pos == m_cameraInfo.m_pivot) return;
	glm::mat4 xRotMatrix = glm::mat4(1.0f);
	glm::mat4 yRotMatrix = glm::mat4(1.0f);
	glm::vec4 tempPos = glm::vec4(m_cameraInfo.m_pos,1.0f);
	if (xRot != 0)
	{
		xRotMatrix = glm::rotate(xRotMatrix, xRot, glm::vec3(0.0f,1.0f,0.0f));
		tempPos = xRotMatrix * (tempPos - glm::vec4(m_cameraInfo.m_pivot, 1.0f)) + glm::vec4(m_cameraInfo.m_pivot,1.0f);
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

glm::mat4& CameraPerspective::calculateTranslations()
{
	m_projMatrix = glm::perspective(glm::radians(m_cameraInfo.m_fov), m_cameraInfo.m_aspectRatio, m_cameraInfo.m_near, m_cameraInfo.m_far);
	m_viewMatrix = m_projMatrix * glm::lookAt(m_cameraInfo.m_pos, m_cameraInfo.m_cameraFace + m_cameraInfo.m_pivot, m_cameraInfo.m_up);
	return m_viewMatrix;
}

PerspectiveCamerInformation CameraPerspective::getCameraInfo()
{
	return m_cameraInfo;
}
void CameraPerspective::setCameraInfo(PerspectiveCamerInformation cameraInfo)
{
	m_cameraInfo = cameraInfo;
}

//not allowed to set view matrix, must be calculated

glm::mat4& CameraPerspective::getViewMatrix()
{
	return m_viewMatrix;
}