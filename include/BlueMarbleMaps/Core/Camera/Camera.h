#ifndef BLUEMARBLE_CAMERA
#define BLUEMARBLE_CAMERA

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Transform.h"

#include "glm.hpp"              // core types: vec3, mat4, etc.
#include "gtc/quaternion.hpp"   // glm::dquat, quaternion operations
#include "gtx/quaternion.hpp"   // quaternion * vector multiplication
#include "gtc/matrix_transform.hpp" // optional, for glm::translate / rotate / lookAt
#include "gtc/constants.hpp"    // optional, for glm::pi etc.
#include "gtc/type_ptr.hpp"     // 
#include "gtx/vec_swizzle.hpp" // for glm::xyz


namespace BlueMarble
{

struct Ray
{
    Point origin;
    Point direction;
};

// Abstract base class for a camera projection.
class CameraProjection
{
public:
    CameraProjection(int width, int height, double near, double far)
        : m_w(width)
        , m_h(height)
        , m_near(near)
        , m_far(far)
    {}

    virtual ~CameraProjection() = default;

    void setViewPort(int width, int height) { m_w = width; m_h = height; };
    double width() const { return m_w; }
    double height() const { return m_h; }
    void setFrustum(double near, double far) { m_near = near; m_far = far; };
    double near() const { return m_near; }
    double far() const { return m_far; }

    Point viewToNdc(const Point& view) const;
    Point ndcToView(const Point& ndc) const;
    Ray ndcToViewRay(const Point& ndc) const;

    double unitsPerPixelAtDistanceNumerical(double zDistCamera) const;
    
    // Must map the principal view point to NDC (0,0)
    virtual glm::dmat4 projectionMatrix() const = 0;
    virtual double unitsPerPixelAtDistance(double zDistCamera) const = 0;

private:
    int   m_w,   m_h;
    double m_near, m_far;
};

class ScreenCameraProjection : public CameraProjection
{
    public:
        ScreenCameraProjection(int width, int height)
            : CameraProjection(width, height, -1.0f, 1.0f)
        {}
    
        glm::dmat4 projectionMatrix() const override final
        {
            return glm::ortho(0.0, (double)width(), (double)height(), 0.0, near(), far()); 
        };

        double unitsPerPixelAtDistance(double zDistCamera) const override final { return 1.0; }
};

class OrthographicCameraProjection : public CameraProjection
{
public:
    OrthographicCameraProjection(int width, int height, double near, double far, double unitsPerPixel)
        : CameraProjection(width, height, near, far)
        , m_unitsPerPixel(unitsPerPixel)
    {}

    glm::dmat4 projectionMatrix() const override final
    {
        double w2 = width() * 0.5 * unitsPerPixel();
        double h2 = height() * 0.5 * unitsPerPixel();
        return glm::ortho(-w2, w2, -h2, h2, near(), far());
    };

    double unitsPerPixelAtDistance(double zDistCamera) const override final { return unitsPerPixel(); }

    double unitsPerPixel() const { return m_unitsPerPixel; }
    void setUnitsPerPixel(double unitsPerPixel) { m_unitsPerPixel = unitsPerPixel; }

private:
    double m_unitsPerPixel;
};

class PerspectiveCameraProjection : public CameraProjection
{
public:
    PerspectiveCameraProjection(int width, int height, double near, double far, double fovDegrees)
        : CameraProjection(width, height, near, far)
        , m_fovDeg(fovDegrees)
    {}

    glm::dmat4 projectionMatrix() const override final
    {
        return glm::perspectiveFov(glm::radians(fov()), (double)width(), (double)height(), near(), far());
    };

    double unitsPerPixelAtDistance(double zDistCamera) const override final { return zDistCamera / focalLengthPixelsY(); }

    double focalLengthPixelsY() const { return height()/(2.0*std::tan(glm::radians(fov() * 0.5))); }

    double fov() const { return m_fovDeg; }
    void setFov(double fovDegrees) { m_fovDeg = fovDegrees; }

private:
    double m_fovDeg;
};

// Forward declaration
class Camera;
using CameraPtr = std::shared_ptr<Camera>;

class Camera
{
public:
    static CameraPtr orthoGraphicCamera(int width, int height, double near, double far, double unitsPerPixel) 
    { 
        return std::make_shared<Camera>(std::make_unique<OrthographicCameraProjection>(width, height, near, far, unitsPerPixel));
    }
    static CameraPtr perspectiveCamera(int width, int height, double near, double far, double fovDegrees)
    {
        return std::make_shared<Camera>(std::make_unique<PerspectiveCameraProjection>(width, height, near, far, fovDegrees));
    }

    Camera(std::unique_ptr<CameraProjection> proj)
    : m_projection(std::move(proj))
    , m_translation(0.0)
    , m_orientation()
    {}

    Camera(std::unique_ptr<CameraProjection> proj, glm::dvec3 translation, glm::dquat orientation)
        : m_projection(std::move(proj))
        , m_translation(translation)
        , m_orientation(orientation)
    {}

    // Projection, these methods are needed for the view
    const std::unique_ptr<CameraProjection>& projection() { return m_projection; }
    void setViewPort(int width, int height) { m_projection->setViewPort(width, height); };
    void setFrustum(double near, double far) { m_projection->setFrustum(near, far); }
    glm::dmat4 projectionMatrix() const { return m_projection->projectionMatrix(); };
    // Optimization for the view to calculate a scale factor for feature queries
    double unitsPerPixelAtDistance(double zDistCamera) const { return m_projection->unitsPerPixelAtDistance(zDistCamera); }

    // Point translation() { return Point(m_transform[3][0], m_transform[3][1], m_transform[3][2]); }
    Point translation() { return Point(m_translation[0], m_translation[1], m_translation[2]); }
    void setTranslation(const Point& t) { m_translation = glm::dvec3(t.x(), t.y(), t.z()); }
    const glm::dquat& orientation() { return m_orientation; }
    void setOrientation(glm::dquat q) { m_orientation = std::move(q); }

    // Add when needed, possibly setOrientationFromForwardUp(forward, up);
    // glm::vec3 forward() { return m_orientation * glm::dvec3(0.0, 0.0, -1.0); }
    // glm::vec3 up() { return m_orientation * glm::dvec3(1.0, 0.0, 1.0); }
    
    glm::dmat4 transform() const;
    glm::dmat4 rotationMatrix() const;
    glm::dmat4 viewMatrix() const { return glm::inverse(std::move(transform())); };
    glm::dmat4 viewProjMatrix() const { return projectionMatrix()*viewMatrix(); };

    Point worldToNdc(const Point& world) const;
    Point worldToView(const Point& world) const;
    Point viewToWorld(const Point& view) const;
    Ray ndcToWorldRay(const Point& ndc) const;

private:
    std::unique_ptr<CameraProjection>   m_projection;
    glm::dvec3                          m_translation;
    glm::dquat                          m_orientation;

    // TODO: abstraction of pose
    // Pose m_pose;
};

} /* BlueMarble */

#endif /* CAMERA */
