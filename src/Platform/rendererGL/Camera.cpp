#include "Camera.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"


Camera::Camera(glm::vec3 position, glm::vec3 cameraDirection, glm::vec3 up, glm::vec3 right, float sensitivity)
   :m_pos(position),
	m_cameraFace(cameraDirection),
	m_up(up),
	m_right(right),
	m_sensitivity(sensitivity)
{

}
Camera::~Camera()
{

}

void Camera::pan(float x, float y, float z)
{
	m_pos += glm::vec3(x, y, z) * m_sensitivity;
}
void Camera::zoom(float zoomFactor)
{

}
void Camera::roll(float rotation)
{

}

glm::mat4 Camera::calculateTranslations()
{
	if(m_directional) m_viewMatrix = m_proj * glm::lookAt(m_pos, m_pos+m_cameraFace, m_up);
	else              m_viewMatrix = m_proj * glm::lookAt(m_pos, m_cameraFace, m_up);
	return m_viewMatrix;
}
void Camera::setProjectionOrtographic(float width, float height, float near, float far)
{
	m_proj = glm::ortho(width,0.0f, 0.0f, height, near, far);
}
void Camera::setProjectionPerspective(float fov, float aspectRatio, float near, float far)
{
	m_proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

glm::mat4 Camera::getViewMatrix()
{
	return m_viewMatrix;
}

glm::vec3 Camera::getPos()
{
	return m_pos;
}
void Camera::setPos(glm::vec3 pos)
{
	m_pos = pos;
}
float Camera::getSensitivity()
{
	return m_sensitivity;
}
void Camera::setSensitivity(float sensitivity)
{
	m_sensitivity = sensitivity;
}
float Camera::getRoll()
{
	return m_roll;
}
void Camera::setRoll(float roll)
{
	m_roll = roll;
}
glm::vec3 Camera::getCameraFace()
{
	return m_cameraFace;
}
void Camera::setCameraFace(glm::vec3 direction)
{
	m_cameraFace = direction;
}
bool Camera::isDirectional()
{
	return m_directional;
}
void Camera::setDirectional(bool directional)
{
	m_directional = directional;
}