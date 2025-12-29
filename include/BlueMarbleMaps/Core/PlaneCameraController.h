#ifndef PLANECAMERACONTROLLER
#define PLANECAMERACONTROLLER

#include "BlueMarbleMaps/Core/ICameraController.h"

namespace BlueMarble
{
class PlaneCameraController : public ICameraController
{
    public:
        PlaneCameraController()
            : m_center(0,0)
            , m_targetCenter(m_center)
            , m_zoom(0.5)
            , m_targetZoom(m_zoom)
            , m_rotation(0.0)
            , m_targetRotation(m_rotation)
            , m_tilt(0.0)
            , m_targetTilt(m_tilt)
            , m_fovDeg(45.0)
            , m_targetFovDeg(m_fovDeg)
            , m_elapsedMs(0)
        {}

        void center(const Point& center) { 
            m_center = center;
            m_targetCenter = center;
        }
        Point center() { return m_center; }
        void zoom(double zoom) { m_zoom = zoom; }
        double zoom() { return m_zoom; }
        void rotation(double rotation) { m_rotation = rotation; }
        double rotation() { return m_rotation; }
        void tilt(double tilt) { m_tilt = tilt; }
        double tilt() { return m_tilt; }
        void fov(double fovDeg) { m_fovDeg=fovDeg; }
        double fov() { return m_fovDeg; }

        
        void panBy(const Point& delta) { 
            m_targetCenter += delta;
            // m_flags |= Panning;
        }
        void zoomBy(double zoomFactor) { 
            m_targetZoom *= zoomFactor; 
            // m_flags |= Zooming;
        }

        void rotateBy(double deltaRot)
        {
            m_targetRotation += deltaRot;
            // m_flags |= Rotating;
        }

        void tiltBy(double deltaTilt)
        {
            m_targetTilt += deltaTilt;
            // m_flags |= Tilting;
        }

        void zoomOn(const Point& point, double zoomFactor)
        {
            // double newScale = m_constraints.constrainValue(scale()*zoomFactor,
            //                                    m_constraints.minScale(),
            //                                    m_constraints.maxScale());
            // zoomFactor = newScale / scale();

            //double actualZoomFactor = m_targetZoom/m_zoom * zoomFactor;

            auto delta = Point((point.x() - m_targetCenter.x()),
                               (point.y() - m_targetCenter.y()));

            auto newCenter = point - delta*(1.0/zoomFactor);

            panBy(newCenter - m_targetCenter);
            zoomBy(zoomFactor);
        }

        void zoomTo(const Rectangle& rect)
        {
            m_targetCenter = rect.center();
            double W = m_camera->projection()->width();
            double w = rect.width();
            double H = m_camera->projection()->height();
            double h = rect.height();
            m_targetZoom = H/h;
            if (W / H < w / h)
                m_targetZoom = W / w;
            else
                m_targetZoom = H / h;

            BMM_DEBUG() << "UNO\n";
            BMM_DEBUG() << "H: " << H << "\n";
            BMM_DEBUG() << "h: " << h << "\n";
            BMM_DEBUG() << "S: " << m_targetZoom << "\n";
            m_targetRotation = 0;
            m_targetTilt = 0;
        }

        CameraPtr onActivated(const CameraPtr& currentCamera, const Rectangle& worldBounds) override final
        {
            // Perspective
            // auto newCamera = Camera::perspectiveCamera(
            //     currentCamera->projection()->width(),
            //     currentCamera->projection()->height(),
            //     currentCamera->projection()->near(),
            //     currentCamera->projection()->far(),
            //     m_fovDeg
            // );
            // Orthographic
            auto newCamera = Camera::orthoGraphicCamera(
                currentCamera->projection()->width(),
                currentCamera->projection()->height(),
                currentCamera->projection()->near(),
                currentCamera->projection()->far(),
                m_zoom
            );

            newCamera->setTransform(currentCamera->transform());

            stateFromCamera(newCamera, worldBounds);

            m_camera = newCamera;

            return newCamera;
        };

        void onDectivated() override final
        {
            m_camera = nullptr;
        };

        ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs) override final
        {
            // Orthographic 2.5D
            // if (m_elapsedMs == 0)
            // {
            //     m_elapsedMs = 1;
            //     deltaMs = 0;
            // }
            constexpr bool animate = true;

            // m_elapsedMs += deltaMs;
            double alpha = deltaMs / 300.0;
            
            // Center
            
            m_center = m_center + (m_targetCenter-m_center)*alpha;
            auto c = m_center;

            // Zoom
            double invFrom = 1.0 / m_zoom;
            double invTo = 1.0 / m_targetZoom;
            double invNew = invFrom + (invTo-invFrom)*alpha;
            double scaleFactor = 1.0 / invNew;
            m_zoom = scaleFactor;
            
            
            // Rotation
            m_rotation = m_rotation + (m_targetRotation-m_rotation)*alpha;
            double rot = m_rotation;

            // Tilt
            m_tilt = m_tilt + (m_targetTilt-m_tilt)*alpha;
            double tilt = m_tilt;

            if (!animate)
            {
                c = m_targetCenter;
                scaleFactor = m_targetZoom;
                rot = m_targetRotation;
                tilt = m_targetTilt;
            }
                

            auto perspective = dynamic_cast<PerspectiveCameraProjection*>(camera->projection().get());
            auto orthographic = dynamic_cast<OrthographicCameraProjection*>(camera->projection().get());

            double z = 1000000.0; // updated for perspective
            if (perspective)
            {
                constexpr bool zoomUsingFov = false;

                perspective->setFov(m_fovDeg);
                double fov = m_fovDeg;
                double focalLength = perspective->focalLengthPixelsY();
                z = float(focalLength / scaleFactor);

                if (zoomUsingFov)
                {
                    z = 100.0; // Fixed z
                    double H = camera->projection()->height();
                    fov = 2* std::atan(H / (scaleFactor*2*z));
                    perspective->setFov(RAD_TO_DEG*fov);
                }
            }
            else if (orthographic)
            {
                orthographic->setUnitsPerPixel(1.0/scaleFactor);
            }
            

            // FIXME: this is a problem since we presume the world surface is z=0
            glm::mat4 cam = glm::mat4(1.0f);
            cam = glm::translate(cam, glm::vec3(
                c.x(),
                c.y(),
                0.0f
            ));
            cam = glm::rotate(cam, (float)glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            cam = glm::rotate(cam, float(glm::radians(tilt)), glm::vec3(1.0f, 0.0f, 0.0f));
            cam = glm::translate(cam, glm::vec3(
                0.0f,
                0.0f,
                z
            ));
            
            camera->setTransform(cam);

            auto status = ControllerStatus::Updated;

            // if ((m_center - m_targetCenter).length() < 1e-6)
            //     m_center = m_targetCenter;
            // else
            //     status = status | ControllerStatus::NeedsUpdate;
            
            // if (std::abs(m_targetZoom - m_zoom) < 1e-6)
            //     m_zoom = m_targetZoom;
            // else
            //     status = status | ControllerStatus::NeedsUpdate;

            // if (std::abs(m_targetRotation - m_rotation) < 1e-6)
            //     m_rotation = m_targetRotation;
            // else
            //     status = status | ControllerStatus::NeedsUpdate;

            // if (std::abs(m_targetTilt - m_tilt) < 1e-6)
            //     m_tilt = m_targetTilt;
            // else
            //     status = status | ControllerStatus::NeedsUpdate;

            if (!hasFlag(status, ControllerStatus::NeedsUpdate))
                m_elapsedMs = 0;

            return status | ControllerStatus::NeedsUpdate;
        };

    private:
        void stateFromCamera(const CameraPtr& camera, const Rectangle& worldBounds)
        {
            m_center = {0,0};
            m_zoom = camera->projection()->width() / worldBounds.width(); // camera->unitsPerPixelAtDistance(1.0);
            m_rotation = 0.0;
            m_tilt = 0.0;

            m_targetCenter = m_center;
            m_targetZoom = m_zoom;
            m_targetRotation = m_rotation;
            m_targetTilt = m_tilt;
        }

        enum InteractionFlags : int
        {
            Panning,
            Zooming,
            Rotating,
            Tilting
        };

        CameraPtr m_camera;
        Point   m_center;
        Point   m_targetCenter;
        double  m_zoom;
        double  m_targetZoom;
        double  m_rotation;
        double  m_targetRotation;
        double  m_tilt;
        double  m_targetTilt;
        // Perspective
        double  m_fovDeg;
        double  m_targetFovDeg;
        
        InteractionFlags m_flags;

        int64_t  m_elapsedMs;
};

}

#endif /* PLANECAMERACONTROLLER */
