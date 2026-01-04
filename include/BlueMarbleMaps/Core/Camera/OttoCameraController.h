#ifndef BLUEMARBLE_OTTOCAMERACONTROLLER
#define BLUEMARBLE_OTTOCAMERACONTROLLER

#include "BlueMarbleMaps/Core/Camera/ICameraController.h"

namespace BlueMarble
{
    struct CameraInformation
    {
        glm::vec3 m_pos = glm::vec3(0.0f,0.0f,35.0f);
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

    struct PerspectiveCamerInformation : CameraInformation
    {
        float m_fov = 45.0f;
        float m_aspectRatio = 1.0f;
    };

    class OttoCameraController : public ICameraController
    {
        public:
            OttoCameraController()
                : m_camera(nullptr)
                , m_cameraInfo()
            {}

            CameraPtr onActivated(const CameraPtr& currentCamera, const SurfaceModelPtr& surfaceModel) override final
            {
                m_camera = currentCamera;

                return currentCamera;
            }

            void onDeactivated() override final
            {
                m_camera = nullptr;
            }

            void panBy(float x, float y, float z=0)
            {
                m_cameraInfo.m_pos += x * m_cameraInfo.m_right;
                m_cameraInfo.m_pivot += x * m_cameraInfo.m_right;
                
                m_cameraInfo.m_pos += y * m_cameraInfo.m_up;
                m_cameraInfo.m_pivot += y * m_cameraInfo.m_up;

                m_cameraInfo.m_pos += z * -m_cameraInfo.m_cameraFace ;
                m_cameraInfo.m_pivot += z * -m_cameraInfo.m_cameraFace;
            }
            void zoomBy(float zoomFactor)
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
            void rollBy(float rotation)
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

            void pitchBy(float rotation)
            {
                if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_right);
                m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
                m_cameraInfo.m_up = glm::cross(-m_cameraInfo.m_cameraFace, m_cameraInfo.m_right);
            }

            void yawBy(float rotation)
            {
                if (m_cameraInfo.m_pivot != m_cameraInfo.m_pos) return;

                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), m_cameraInfo.m_up);
                m_cameraInfo.m_cameraFace = glm::normalize(glm::mat3(rotationMatrix) * m_cameraInfo.m_cameraFace);
                m_cameraInfo.m_right = glm::normalize(glm::cross(m_cameraInfo.m_cameraFace, m_cameraInfo.m_up));
            }

            Point translation() { return Point(m_cameraInfo.m_pos.x, m_cameraInfo.m_pos.y, m_cameraInfo.m_pos.z); }

            void setPivot(const Point& pivot)
            {
                m_cameraInfo.m_pivot = glm::vec3(pivot.x(), pivot.y(), pivot.z());
            }

            void orbitBy(float xRot, float yRot)
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

            glm::mat4 calculateTransform()
            {
                // m_projMatrix = glm::perspective(glm::radians(m_cameraInfo.m_fov), m_cameraInfo.m_aspectRatio, m_cameraInfo.m_near, m_cameraInfo.m_far);
                auto viewMat = glm::lookAt(m_cameraInfo.m_pos, m_cameraInfo.m_cameraFace + m_cameraInfo.m_pivot, m_cameraInfo.m_up);
                return glm::inverse(viewMat);
            }
            
            bool needsUpdate() const override final
            {
                return true;
            }

            virtual ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs) override final
            {
                auto perspective = dynamic_cast<PerspectiveCameraProjection*>(camera->projection().get());
                if (perspective)
                {
                    perspective->setFov(m_cameraInfo.m_fov);
                }
                camera->setTransform(calculateTransform());

                return ControllerStatus::Updated;
            }

        private:
            CameraPtr m_camera;

            PerspectiveCamerInformation m_cameraInfo;
    };
}

#endif /* BLUEMARBLE_OTTOCAMERACONTROLLER */
