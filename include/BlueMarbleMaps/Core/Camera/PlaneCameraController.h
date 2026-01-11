#ifndef PLANECAMERACONTROLLER
#define PLANECAMERACONTROLLER

#include "BlueMarbleMaps/Core/Camera/ICameraController.h"

namespace BlueMarble
{

enum InteractionFlags : int
{
    ControllerIdle        = 0,
    ControllerPanning     = BIT(0),
    ControllerZooming     = BIT(1),
    ControllerRotating    = BIT(2),
    ControllerTilting     = BIT(3),
    ControllerChangingFov = BIT(4)
};

inline InteractionFlags operator|(InteractionFlags a, InteractionFlags b)
{
    return static_cast<InteractionFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline InteractionFlags operator&(
    InteractionFlags a,
    InteractionFlags b)
{
    return static_cast<InteractionFlags>(
        static_cast<uint32_t>(a) &
        static_cast<uint32_t>(b));
}

inline InteractionFlags operator~(
    InteractionFlags a)
{
    return static_cast<InteractionFlags>(~static_cast<uint32_t>(a));
}

constexpr bool hasFlag(InteractionFlags value, InteractionFlags flag)
{
    using T = std::underlying_type_t<InteractionFlags>;
    return (static_cast<T>(value) & static_cast<T>(flag)) != 0;
}

class PlaneCameraController : public ICameraController
{
    public:
        PlaneCameraController()
            : m_center(0,0)
            , m_targetCenter(m_center)
            , m_currentWorldBounds(Rectangle::undefined())
            , m_zoom(1.0)
            , m_targetZoom(m_zoom)
            , m_rotation(0.0)
            , m_targetRotation(m_rotation)
            , m_tilt(0.0)
            , m_targetTilt(m_tilt)
            , m_maxTilt(90.0)
            , m_minTilt(-90.0)
            , m_fovDeg(75.0)
            , m_targetFovDeg(m_fovDeg)
            , m_justStarted(true)
            , m_elapsedMs(0)
            , m_flags(InteractionFlags::ControllerIdle)
        {}

        void center(const Point& center) 
        { 
            if (!m_currentWorldBounds.isInside(center))
            {
                double newX = Utils::clampValue(center.x(), m_currentWorldBounds.xMin(), m_currentWorldBounds.xMax());
                double newY = Utils::clampValue(center.y(), m_currentWorldBounds.yMin(), m_currentWorldBounds.yMax());
                m_center = Point(newX, newY);
                m_targetCenter = center;
                return;
            }
            m_center = center;
            m_targetCenter = center;
        }
        Point center() { return m_center; }
        void zoom(double zoom) { m_zoom = zoom; m_targetZoom = m_zoom; }
        double zoom() { return m_zoom; }
        void rotation(double rotation) { m_rotation = rotation; m_targetRotation = m_rotation; }
        double rotation() { return m_rotation; }
        void tilt(double tilt) { m_tilt = Utils::clampValue(tilt, m_minTilt, m_maxTilt); m_targetTilt = m_tilt; }
        double tilt() { return m_tilt; }
        void fov(double fovDeg) { m_fovDeg=fovDeg; }
        double fov() { return m_fovDeg; }

        
        void panBy(const Point& delta) 
        { 
            m_targetCenter += delta;
            if (!m_currentWorldBounds.isInside(m_targetCenter))
            {
                double newX = Utils::clampValue(m_targetCenter.x(), m_currentWorldBounds.xMin(), m_currentWorldBounds.xMax());
                double newY = Utils::clampValue(m_targetCenter.y(), m_currentWorldBounds.yMin(), m_currentWorldBounds.yMax());
                m_targetCenter = Point(newX, newY);
            }
            if (m_flags == InteractionFlags::ControllerIdle) m_justStarted = true;
            m_flags = m_flags | ControllerPanning;
        }
        void zoomBy(double zoomFactor) 
        { 
            m_targetZoom *= zoomFactor; 
            if (m_flags == InteractionFlags::ControllerIdle) m_justStarted = true;
            m_flags = m_flags | ControllerZooming;
        }

        void rotateBy(double deltaRot)
        {
            m_targetRotation += deltaRot;
            if (m_flags == InteractionFlags::ControllerIdle) m_justStarted = true;
            m_flags = m_flags | ControllerRotating;
        }

        void tiltBy(double deltaTilt)
        {
            m_targetTilt += deltaTilt;
            m_targetTilt = Utils::clampValue(m_targetTilt, m_minTilt, m_maxTilt);
            m_targetTilt = Utils::clampValue(m_targetTilt, m_targetFovDeg/2.0-90.0 + 0.0001, 90.0-m_targetFovDeg/2.0 - 0.0001); // Such that the camera does not look "beyond" the plane
            if (m_flags == InteractionFlags::ControllerIdle) m_justStarted = true;
            m_flags = m_flags | ControllerTilting;
        }

        void zoomOn(const Point& point, double zoomFactor)
        {
            auto delta = Point((point.x() - m_targetCenter.x()),
                               (point.y() - m_targetCenter.y()));

            auto newCenter = point - delta*(1.0/zoomFactor);

            panBy(newCenter - m_targetCenter);
            zoomBy(zoomFactor);
        }

        void zoomTo(const Rectangle& rect)
        {
            // Pan
            panBy(rect.center() - m_targetCenter);
            double W = m_camera->projection()->width();
            double w = rect.width();
            double H = m_camera->projection()->height();
            double h = rect.height();

            // Zoom
            if (W / H < w / h)
            {
                zoomBy(W / w / m_targetZoom);
            }
            else
            {
                zoomBy(H / h / m_targetZoom);
            }

            // Rotate/Tilt
            rotateBy(-m_targetRotation);
            tiltBy(-m_targetTilt);
        }

        void changeFovBy(double deltaDegrees)
        {
            m_targetFovDeg += deltaDegrees;
            m_targetFovDeg = Utils::clampValue(m_targetFovDeg, 1.0, 179.0);
            if (m_flags == InteractionFlags::ControllerIdle) m_justStarted = true;
            m_flags = m_flags | ControllerChangingFov;

            // When changing fov, wee need to take care of tilt such that 
            // the camera does not look "beyond" the horizon
            tiltBy(0.0);
        }

        CameraPtr onActivated(const CameraPtr& currentCamera, const CrsPtr& crs, const SurfaceModelPtr& surfaceModel) override final
        {
            constexpr bool usePerspective = true;
            CameraPtr newCamera;

            if (usePerspective)
            {
                // Perspective
                newCamera = Camera::perspectiveCamera(
                    currentCamera->projection()->width(),
                    currentCamera->projection()->height(),
                    currentCamera->projection()->near(),
                    currentCamera->projection()->far(),
                    m_fovDeg
                );
            }
            else
            {
                // Orthographic
                newCamera = Camera::orthoGraphicCamera(
                    currentCamera->projection()->width(),
                    currentCamera->projection()->height(),
                    currentCamera->projection()->near(),
                    currentCamera->projection()->far(),
                    m_zoom
                );
            }

            newCamera->setTranslation(currentCamera->translation());
            newCamera->setOrientation(currentCamera->orientation());
            // newCamera->setTransform(currentCamera->transform());

            stateFromCamera(newCamera, crs);

            m_camera = newCamera;

            return newCamera;
        };

        void onDeactivated() override final
        {
            m_camera = nullptr;
        };

        ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs) override final
        {
            if (m_flags == InteractionFlags::ControllerIdle)
                return ControllerStatus::Idle;

            if (m_justStarted)
            {
                deltaMs = 10; // TODO: need to fix time context
                m_justStarted = false;
            }
            constexpr bool animate = true;

            // m_elapsedMs += deltaMs;
            double alpha = deltaMs / 100.0;
            alpha = std::min(alpha, 1.0);
            
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
            m_rotation = m_rotation + Utils::minAngleDiff(m_targetRotation, m_rotation, 0.0, 360.0)*alpha;
            double rot = m_rotation;

            // Tilt
            m_tilt = m_tilt + Utils::minAngleDiff(m_targetTilt, m_tilt, -180.0, 180.0)*alpha;
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

            double z = 1.0;
            if (perspective)
            {
                constexpr bool zoomUsingFov = false;

                m_fovDeg += (m_targetFovDeg-m_fovDeg)*alpha;
                perspective->setFov(m_fovDeg);
                double fov = m_fovDeg;
                double focalLength = perspective->focalLengthPixelsY();
                z = double(focalLength / scaleFactor);

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
                double H = camera->projection()->height();
                double zWorld = H/scaleFactor;
                z = zWorld / std::cos(glm::radians(m_tilt));
            }
            
            glm::dvec3 center = glm::dvec3(c.x(), c.y(), 0.0); // the pivot/world point camera looks at
            double rotRad  = glm::radians(rot);  // yaw
            double tiltRad = glm::radians(tilt); // pitch
            double distance = z;                  // camera distance from center along forward axis

            // Build orientation quaternion (yaw then pitch)
            glm::dquat qYaw   = glm::angleAxis(rotRad,  glm::dvec3(0.0, 0.0, 1.0));
            glm::dquat qPitch = glm::angleAxis(tiltRad, glm::dvec3(1.0, 0.0, 0.0));
            glm::dquat orientation = qYaw * qPitch;

            // Compute forward vector in world space
            glm::dvec3 forward = orientation * glm::dvec3(0.0, 0.0, -1.0);

            // Camera position = pivot minus forward * distance
            glm::dvec3 translation = center - forward * distance;

            camera->setOrientation(orientation);
            camera->setTranslation(Point(translation.x, translation.y, translation.z));

            auto status = ControllerStatus::Updated;

            if (m_flags & InteractionFlags::ControllerPanning &&
                (m_center - m_targetCenter).length() < 1e-6)
            {
                m_center = m_targetCenter;
                m_flags = m_flags & ~InteractionFlags::ControllerPanning;
                BMM_DEBUG() << "Stopped Panning\n";
                // logFlags();
            }
            
            if (m_flags & InteractionFlags::ControllerZooming &&
                std::abs(m_targetZoom - m_zoom) < 1e-10)
            {
                m_zoom = m_targetZoom;
                m_flags = m_flags & ~InteractionFlags::ControllerZooming;
                BMM_DEBUG() << "Stopped Zooming\n";
                // logFlags();
            }

            if (m_flags & InteractionFlags::ControllerRotating &&
                std::abs(Utils::minAngleDiff(m_targetRotation, m_rotation, 0.0, 360.0)) < 1e-6)
            {
                m_rotation = m_targetRotation;
                m_flags = m_flags & ~InteractionFlags::ControllerRotating;
                BMM_DEBUG() << "Stopped Rotating\n";
                // logFlags();
            }

            if (m_flags & InteractionFlags::ControllerTilting &&
                std::abs(Utils::minAngleDiff(m_targetTilt, m_tilt, 0.0, 360.0)) < 1e-6)
            {
                m_tilt = m_targetTilt;
                m_flags = m_flags & ~InteractionFlags::ControllerTilting;
                BMM_DEBUG() << "Stopped Tilting\n";
                // logFlags();
            }

            if (m_flags & InteractionFlags::ControllerChangingFov &&
                std::abs(m_targetFovDeg - m_fovDeg) < 1e-6)
            {
                m_fovDeg = m_targetFovDeg;
                m_flags = m_flags & ~InteractionFlags::ControllerChangingFov;
                BMM_DEBUG() << "Stopped Changing fov\n";
                // logFlags();
            }

            if (m_flags == InteractionFlags::ControllerIdle)
            {
                BMM_DEBUG() << "IDLE!\n";
            }

            return m_flags == InteractionFlags::ControllerIdle ? ControllerStatus::Idle : ControllerStatus::NeedsUpdate;
        };

        void stop()
        {
            m_targetCenter = m_center;
            m_targetZoom = m_zoom;
            m_targetRotation = m_rotation;
            m_targetTilt = m_tilt;

            m_flags = InteractionFlags::ControllerIdle;
        }

    private:
        void stateFromCamera(const CameraPtr& camera, const CrsPtr& crs)
        {
            if (m_currentWorldBounds.isUndefined())
            {
                auto worldBounds = crs->bounds();
                m_center = Point(worldBounds.center());
                // TODO: which to use?
                double zW = camera->projection()->width() / worldBounds.width();
                double zH = camera->projection()->height() / worldBounds.height();
                m_zoom = std::min(zW, zH);
            }
            else
            {
                m_center = m_crs->projectTo(crs, m_center);
                m_zoom *= crs->globalMeterScale() / m_crs->globalMeterScale();
            }

            m_crs = crs;
            m_targetCenter = m_center;
            m_targetZoom = m_zoom;
            m_targetRotation = m_rotation;
            m_targetTilt = m_tilt;
            m_currentWorldBounds = crs->bounds();

            panBy({0.0,0.0}); // Just to trigger update
        }

        void logFlags()
        {
            BMM_DEBUG() << std::boolalpha;
            BMM_DEBUG() << "Panning: " << hasFlag(m_flags, InteractionFlags::ControllerPanning) << "\n";
            BMM_DEBUG() << "Zooming: " << hasFlag(m_flags, InteractionFlags::ControllerZooming) << "\n";
            BMM_DEBUG() << "Rotating: " << hasFlag(m_flags, InteractionFlags::ControllerRotating) << "\n";
            BMM_DEBUG() << "Tilting: " << hasFlag(m_flags, InteractionFlags::ControllerTilting) << "\n";
            BMM_DEBUG() << "Idle: " << (m_flags == InteractionFlags::ControllerIdle) << "\n";
            BMM_DEBUG() << "Flags: " << m_flags << "\n";
        }

        CameraPtr m_camera;
        CrsPtr    m_crs;
        
        Point   m_center;
        Point   m_targetCenter;
        Rectangle m_currentWorldBounds;
        
        double  m_zoom;
        double  m_targetZoom;
        
        double  m_rotation;
        double  m_targetRotation;

        double  m_tilt;
        double  m_targetTilt;
        double  m_maxTilt;
        double  m_minTilt;
        // Perspective
        double  m_fovDeg;
        double  m_targetFovDeg;

        bool    m_justStarted;
        InteractionFlags m_flags;
        int64_t  m_elapsedMs;
};

}

#endif /* PLANECAMERACONTROLLER */
