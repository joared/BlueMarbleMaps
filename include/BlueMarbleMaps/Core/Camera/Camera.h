#ifndef BLUEMARBLE_CAMERA
#define BLUEMARBLE_CAMERA

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Transform.h"

#include "glm/include/glm.hpp"
#include "glm/include/ext/matrix_clip_space.hpp"
#include "glm/include/ext/matrix_transform.hpp"
#include "glm/include/gtx/vec_swizzle.hpp"

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
    CameraProjection(int width, int height, float near, float far)
        : m_w(width)
        , m_h(height)
        , m_near(near)
        , m_far(far)
    {}

    virtual ~CameraProjection() = default;

    void setViewPort(int width, int height) { m_w = width; m_h = height; };
    float width() const { return m_w; }
    float height() const { return m_h; }
    void setFrustum(float near, float far) { m_near = near; m_far = far; };
    float near() const { return m_near; }
    float far() const { return m_far; }

    Point viewToNdc(const Point& view) const;
    Point ndcToView(const Point& ndc) const;
    Ray ndcToViewRay(const Point& ndc) const;

    double unitsPerPixelAtDistanceNumerical(double zDistCamera) const;
    
    // Must map the principal view point to NDC (0,0)
    virtual glm::mat4 projectionMatrix() const = 0;
    virtual double unitsPerPixelAtDistance(double zDistCamera) const = 0;

private:
    int   m_w,   m_h;
    float m_near, m_far;
};

class ScreenCameraProjection : public CameraProjection
{
    public:
        ScreenCameraProjection(int width, int height)
            : CameraProjection(width, height, -1.0f, 1.0f)
        {}
    
        glm::mat4 projectionMatrix() const override final
        {
            return glm::ortho(0.0f, (float)width(), (float)height(), 0.0f, near(), far()); 
        };

        double unitsPerPixelAtDistance(double zDistCamera) const override final { return 1.0; }
};

class OrthographicCameraProjection : public CameraProjection
{
public:
    OrthographicCameraProjection(int width, int height, float near, float far, float unitsPerPixel)
        : CameraProjection(width, height, near, far)
        , m_unitsPerPixel(unitsPerPixel)
    {}

    glm::mat4 projectionMatrix() const override final
    {
        float w2 = width() * 0.5 * unitsPerPixel();
        float h2 = height() * 0.5 * unitsPerPixel();
        return glm::ortho(-w2, w2, -h2, h2, near(), far());
    };

    double unitsPerPixelAtDistance(double zDistCamera) const override final { return unitsPerPixel(); }

    float unitsPerPixel() const { return m_unitsPerPixel; }
    void setUnitsPerPixel(float unitsPerPixel) { m_unitsPerPixel = unitsPerPixel; }

private:
    float m_unitsPerPixel;
};

class PerspectiveCameraProjection : public CameraProjection
{
public:
    PerspectiveCameraProjection(int width, int height, float near, float far, float fovDegrees)
        : CameraProjection(width, height, near, far)
        , m_fovDeg(fovDegrees)
    {}

    glm::mat4 projectionMatrix() const override final
    {
        return glm::perspectiveFov(glm::radians(fov()), (float)width(), (float)height(), near(), far());
    };

    double unitsPerPixelAtDistance(double zDistCamera) const override final { return zDistCamera / focalLengthPixelsY(); }

    float focalLengthPixelsY() const { return height()/(2.0*std::tan(glm::radians(fov() * 0.5))); }

    float fov() const { return m_fovDeg; }
    void setFov(float fovDegrees) { m_fovDeg = fovDegrees; }

private:
    float m_fovDeg;
};

// Forward declaration
class Camera;
using CameraPtr = std::shared_ptr<Camera>;

class Camera
{
public:
    static CameraPtr orthoGraphicCamera(int width, int height, float near, float far, float unitsPerPixel) 
    { 
        return std::make_shared<Camera>(std::make_unique<OrthographicCameraProjection>(width, height, near, far, unitsPerPixel));
    }
    static CameraPtr perspectiveCamera(int width, int height, float near, float far, float fovDegrees)
    {
        return std::make_shared<Camera>(std::make_unique<PerspectiveCameraProjection>(width, height, near, far, fovDegrees));
    }

    Camera(std::unique_ptr<CameraProjection> proj)
    : m_projection(std::move(proj))
    , m_transform(glm::mat4(1.0f))

    {}
    Camera(std::unique_ptr<CameraProjection> proj, glm::mat4 transform)
        : m_projection(std::move(proj))
        , m_transform(std::move(transform))

    {}

    // Projection, these methods are needed for the view
    const std::unique_ptr<CameraProjection>& projection() { return m_projection; }
    void setViewPort(int width, int height) { m_projection->setViewPort(width, height); };
    void setFrustum(float near, float far) { m_projection->setFrustum(near, far); }
    glm::mat4 projectionMatrix() const { return m_projection->projectionMatrix(); };
    // Optimization for the view to calculate a scale factor for feature queries
    double unitsPerPixelAtDistance(double zDistCamera) const { return m_projection->unitsPerPixelAtDistance(zDistCamera); }

    Point translation() { return Point(m_transform[3][0], m_transform[3][1], m_transform[3][2]); }
    glm::vec3 forward() { return -glm::normalize(glm::vec3(m_transform[2])); }
    glm::vec3 up() { return glm::normalize(glm::vec3(m_transform[1])); }
    
    glm::mat4 transform() const { return m_transform; };
    void setTransform(glm::mat4 t) { m_transform = std::move(t); };
    glm::mat4 viewMatrix() const { return glm::inverse(m_transform); };
    glm::mat4 viewProjMatrix() const { return projectionMatrix()*viewMatrix(); };

    Point worldToNdc(const Point& world) const;
    Point worldToView(const Point& world) const;
    Point viewToWorld(const Point& view) const;
    Ray ndcToWorldRay(const Point& ndc) const;
    // Point ndcToWorld(const Point& ndc) const; // TODO: Needs surface to work

private:
    std::unique_ptr<CameraProjection>   m_projection;
    glm::mat4                           m_transform;

    // TODO: abstraction of pose
    // Pose m_pose;
};

} /* BlueMarble */

#endif /* CAMERA */
