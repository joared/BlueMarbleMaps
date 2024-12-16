#include "Camera2D.h"

Camera2D::Camera2D(glm::vec3 up, glm::vec3 right)
{

}
Camera2D::~Camera2D()
{

}

void Camera2D::pan(int x, int y)
{

}
void Camera2D::zoom(float zoomFactor)
{

}
void Camera2D::roll(float rotation)
{
	
}

void Camera2D::calculateTranslations()
{

}
glm::mat4 Camera2D::getViewMatrix()
{
	return m_viewMatrix;
}

glm::vec3 Camera2D::getPos()
{
	return m_pos;
}
void Camera2D::setPos(glm::vec3 pos)
{
	m_pos = pos;
}
float Camera2D::getSensitivity()
{
	return m_sensitivity;
}
void Camera2D::setSensitivity(float sensitivity)
{
	m_sensitivity = sensitivity;
}
float Camera2D::getRoll()
{
	return m_roll;
}
void Camera2D::setRoll(float roll)
{
	m_roll = roll;
}