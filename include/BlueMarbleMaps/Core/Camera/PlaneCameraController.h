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

class PlaneCameraController : public ICameraNavigator
{
    public:
        PlaneCameraController()
            : m_center(0,0)
            , m_targetCenter(m_center)
            , m_currentWorldBounds(Rectangle::undefined())
            , m_zoom(1.0)
            , m_targetZoom(m_zoom)
            , m_maxZoom(1.0)
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

        
        void panTo(const Point& target) override final
        {
            panBy(target - m_targetCenter);

            setSlowResponseTime();
        }

        void panBy(const Point& delta) 
        { 
            m_targetCenter += delta;
            if (!m_currentWorldBounds.isInside(m_targetCenter))
            {
                double newX = Utils::clampValue(m_targetCenter.x(), m_currentWorldBounds.xMin(), m_currentWorldBounds.xMax());
                double newY = Utils::clampValue(m_targetCenter.y(), m_currentWorldBounds.yMin(), m_currentWorldBounds.yMax());
                m_targetCenter = Point(newX, newY);
            }

            applyInteraction(ControllerPanning);
            setFastResponseTime();
        }
        void zoomBy(double zoomFactor) 
        { 
            m_targetZoom *= zoomFactor;
            m_targetZoom = Utils::clampValue(m_targetZoom, 0.0, m_maxZoom);

            applyInteraction(ControllerZooming);
            setFastResponseTime();
        }

        void rotateBy(double deltaRot)
        {
            m_targetRotation += deltaRot;

            applyInteraction(ControllerRotating);
            setFastResponseTime();
        }

        void tiltBy(double deltaTilt)
        {
            m_targetTilt += deltaTilt;
            m_targetTilt = Utils::clampValue(m_targetTilt, m_minTilt, m_maxTilt);
            m_targetTilt = Utils::clampValue(m_targetTilt, m_targetFovDeg/2.0-90.0 + 0.0001, 90.0-m_targetFovDeg/2.0 - 0.0001); // Such that the camera does not look "beyond" the plane

            applyInteraction(ControllerTilting);
            setFastResponseTime();
        }

        void applyInteraction(InteractionFlags flags)
        {
            if (m_flags == InteractionFlags::ControllerIdle) 
            {
                m_justStarted = true;
            }
            m_elapsedMs = 0;
            m_flags = m_flags | flags;
        }

        void zoomOn(const Point& point, double zoomFactor)
        {
            auto delta = Point((point.x() - m_targetCenter.x()),
                               (point.y() - m_targetCenter.y()));

            auto newCenter = point - delta*(1.0/zoomFactor);

            panBy(newCenter - m_targetCenter);
            zoomBy(zoomFactor);

            setFastResponseTime();
        }

        void rotateTo(double rotation) override final
        {
            rotateBy(-Utils::minAngleDiff(m_targetRotation, rotation, 0.0, 360));
            setMediumResponseTime();
        }

        void zoomTo(const Rectangle& rect) override final
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

            setSlowResponseTime();
        }

        void changeFovBy(double deltaDegrees)
        {
            m_targetFovDeg += deltaDegrees;
            m_targetFovDeg = Utils::clampValue(m_targetFovDeg, 1.0, 179.0);
            
            applyInteraction(ControllerChangingFov);

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

            stateFromCamera(newCamera, crs);

            m_camera = newCamera;

            return newCamera;
        };

        void onDeactivated() override final
        {
            m_camera = nullptr;
        };

        void onCrsChanged(const CrsPtr& crs) override final
        {
            if (!m_camera) return;
            stateFromCamera(m_camera, crs);
        }

        void onSurfaceModelChanged(const SurfaceModelPtr& surfaceModel) override final
        {
            if (!m_camera) return;
            stateFromCamera(m_camera, m_crs);
        }

        void onViewportSizeChanged(int width, int height) override final
        {
            if (!m_camera) return;
            stateFromCamera(m_camera, m_crs);
        }

        ControllerStatus updateCamera(const CameraPtr& camera, int64_t deltaMs) override final
        {
            constexpr bool animate = true;

            if (m_flags == InteractionFlags::ControllerIdle)
                return ControllerStatus::Idle;

            if (animate)
            {
                if (m_justStarted)
                {
                    deltaMs = 10; // TODO: need to fix time context
                    m_elapsedMs = 0;
                    m_justStarted = false;
                }
                m_elapsedMs += deltaMs;

                double alpha = calculateAlpha(deltaMs);
                
                updateCenter(alpha);
                updateZoom(alpha);
                updateRotation(alpha);
                updateTilt(alpha);
                updateFov(camera->projection().get(), alpha);

                updateCameraPose(camera);

                if (alpha >= 1.0) 
                    stop();
            }
            else
            {
                m_center = m_targetCenter;
                m_zoom = m_targetZoom;
                m_rotation = m_targetRotation;
                m_tilt = m_targetTilt;
                m_fovDeg = m_targetFovDeg;

                m_flags = InteractionFlags::ControllerIdle;
            }

            if (m_flags == InteractionFlags::ControllerIdle)
            {
                BMM_DEBUG() << "IDLE!\n";
            }

            return m_flags == InteractionFlags::ControllerIdle ? ControllerStatus::Idle : ControllerStatus::NeedsUpdate;
        };

        void stop()
        {
            m_justStarted = false;
            m_elapsedMs = 0;
            m_targetCenter = m_center;
            m_targetZoom = m_zoom;
            m_targetRotation = m_rotation;
            m_targetTilt = m_tilt;

            m_flags = InteractionFlags::ControllerIdle;
        }

    private:

        double calculateAlpha(int64_t deltaMs)
        {
            double alpha = (double)deltaMs/m_responseTimeMs;

            // The per-frame ease above asymptotically approaches the target but
            // never algebraically reaches it, so callers rely on an absolute
            // distance epsilon to detect convergence. That breaks down for
            // large-magnitude coordinates (e.g. Web Mercator meters), where the
            // remaining delta can shrink below the double's ULP at that
            // magnitude before it shrinks below the epsilon, freezing the
            // animation forever. Force a hard, time-bound convergence instead:
            // once several response-time constants worth of time has elapsed,
            // the exponential ease is visually indistinguishable from done, so
            // snap alpha to 1.0 regardless of the remaining distance.
            constexpr double convergenceFactor = 8.0; // (1 - 1/e)^8 leaves < 0.04% remaining
            if (m_elapsedMs >= convergenceFactor * m_responseTimeMs)
                alpha = 1.0;

            alpha = std::min(alpha, 1.0);

            return alpha;
        }

        void updateCenter(double alpha)
        {
            m_center = m_center + (m_targetCenter-m_center)*alpha;

            if (m_flags & InteractionFlags::ControllerPanning &&
                (m_center - m_targetCenter).length() < 1e-9)
            {
                m_center = m_targetCenter;
                m_flags = m_flags & ~InteractionFlags::ControllerPanning;
                // BMM_DEBUG() << "Stopped Panning\n";
                // logFlags();
            }
        }

        void updateZoom(double alpha)
        {
            double invFrom = 1.0 / m_zoom;
            double invTo = 1.0 / m_targetZoom;
            double invNew = invFrom + (invTo-invFrom)*alpha;
            double scaleFactor = 1.0 / invNew;
            m_zoom = scaleFactor;

            if (m_flags & InteractionFlags::ControllerZooming &&
                std::abs(m_targetZoom - m_zoom) < 1e-10)
            {
                m_zoom = m_targetZoom;
                m_flags = m_flags & ~InteractionFlags::ControllerZooming;
                // BMM_DEBUG() << "Stopped Zooming\n";
                // logFlags();
            }
        }

        void updateRotation(double alpha)
        {
            m_rotation = m_rotation + Utils::minAngleDiff(m_targetRotation, m_rotation, 0.0, 360.0)*alpha;

            if (m_flags & InteractionFlags::ControllerRotating &&
                std::abs(Utils::minAngleDiff(m_targetRotation, m_rotation, 0.0, 360.0)) < 1e-6)
            {
                m_rotation = m_targetRotation;
                m_flags = m_flags & ~InteractionFlags::ControllerRotating;
                // BMM_DEBUG() << "Stopped Rotating\n";
                // logFlags();
            }
        }

        void updateTilt(double alpha)
        {
            m_tilt = m_tilt + Utils::minAngleDiff(m_targetTilt, m_tilt, -180.0, 180.0)*alpha;

            if (m_flags & InteractionFlags::ControllerTilting &&
                std::abs(Utils::minAngleDiff(m_targetTilt, m_tilt, 0.0, 360.0)) < 1e-6)
            {
                m_tilt = m_targetTilt;
                m_flags = m_flags & ~InteractionFlags::ControllerTilting;
                // BMM_DEBUG() << "Stopped Tilting\n";
                // logFlags();
            }
        }

        double calculateDistanceToCenter(CameraProjection* projection)
        {
            auto perspective = dynamic_cast<PerspectiveCameraProjection*>(projection);
            auto orthographic = dynamic_cast<OrthographicCameraProjection*>(projection);

            if (perspective)
            {
                constexpr bool zoomUsingFov = false;

                perspective->setFov(m_fovDeg);
                double fov = m_fovDeg;
                double focalLength = perspective->focalLengthPixelsY();
                return focalLength / m_zoom;

                if (zoomUsingFov)
                {
                    return 100.0; // Fixed z
                }
            }
            else if (orthographic)
            {
                double zWorld = projection->height()/m_zoom;
                return zWorld / std::cos(glm::radians(m_tilt));
            }

            return 1.0;
        }

        double updateFov(CameraProjection* projection, double alpha)
        {
            m_fovDeg += (m_targetFovDeg-m_fovDeg)*alpha;

            if (m_flags & InteractionFlags::ControllerChangingFov &&
                std::abs(m_targetFovDeg - m_fovDeg) < 1e-6)
            {
                m_fovDeg = m_targetFovDeg;
                m_flags = m_flags & ~InteractionFlags::ControllerChangingFov;
                // BMM_DEBUG() << "Stopped Changing fov\n";
                // logFlags();
            }
        }

        void updateCameraPose(const CameraPtr camera)
        {
            constexpr bool ZoomUsingFov = false;

            glm::dvec3 center = glm::dvec3(m_center.x(), m_center.y(), 0.0); // the pivot/world point camera looks at
            double rotRad  = glm::radians(m_rotation);  // yaw
            double tiltRad = glm::radians(m_tilt); // pitch
            double distance = calculateDistanceToCenter(camera->projection().get()); // camera distance from center along forward axis

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

            auto perspective = dynamic_cast<PerspectiveCameraProjection*>(camera->projection().get());
            auto orthographic = dynamic_cast<OrthographicCameraProjection*>(camera->projection().get());

            if (perspective)
            {
                perspective->setFov(m_fovDeg);
                if (ZoomUsingFov)
                {
                    constexpr double fixedDistance = 100.0; // I dont remember why
                    double H = perspective->height();
                    double fov = 2* std::atan(H / (m_zoom*2*fixedDistance));
                    perspective->setFov(RAD_TO_DEG*fov);
                }
            }
            else if (orthographic)
            {
                orthographic->setUnitsPerPixel(1.0/m_zoom);
            }
        }

        void setFastResponseTime()
        {
            m_responseTimeMs = 150.0;
        }
        void setMediumResponseTime()
        {
            m_responseTimeMs = 300.0;
        }
        void setSlowResponseTime()
        {
            m_responseTimeMs = 500.0;
        }

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
                m_zoom *= crs->globalMetersPerUnit() / m_crs->globalMetersPerUnit();
            }

            m_crs = crs;
            m_targetCenter = m_center;
            m_targetZoom = m_zoom;
            m_targetRotation = m_rotation;
            m_targetTilt = m_tilt;
            m_currentWorldBounds = crs->bounds();
            m_maxZoom = crs->globalMetersPerUnit() * 100.0; // Result in 1/100 meters per pixel

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
        double  m_maxZoom;
        
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

        double m_responseTimeMs;
};

}

#endif /* PLANECAMERACONTROLLER */
