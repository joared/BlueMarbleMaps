#include "BlueMarbleMaps/Core/Camera/Camera.h"


using namespace BlueMarble;

Point CameraProjection::viewToNdc(const Point &view) const
{
    auto proj = projectionMatrix();

    glm::dvec4 p = glm::dvec4((double)view.x(), (double)view.y(), (double)view.z(), 1.0);
    glm::dvec4 clipSpace = proj * p;
    glm::dvec3 ndc = glm::xyz(clipSpace) / clipSpace.w;

    return Point(ndc.x, ndc.y, ndc.z);
}

Point CameraProjection::ndcToView(const Point& ndc) const
{
    auto proj = projectionMatrix();

    glm::dvec4 ndcGlm(ndc.x(), ndc.y(), ndc.z(), 1.0);
    glm::dvec4 homo = glm::inverse(proj) * ndcGlm;
    glm::dvec3 view = glm::xyz(homo / homo.w);

    return Point(view.x, view.y, view.z);
}

Ray CameraProjection::ndcToViewRay(const Point& ndc) const
{
    // Ignore the z value and choose the point on the near plane (-1.0)
    Point near = ndcToView({ndc.x(), ndc.y(), -1.0});
    Point far = ndcToView({ndc.x(), ndc.y(), 1.0});
    Ray ray;
    ray.origin = near;
    ray.direction = (far - near).norm3D();
    
    return ray;
}

glm::dmat4 Camera::transform() const
{
    glm::dmat4 transMatrix = glm::translate(glm::dmat4(1.0), m_translation);
    glm::dmat4 cam = transMatrix * rotationMatrix(); // Or other order as needed

    return cam;
}

glm::dmat4 Camera::rotationMatrix() const
{
    return glm::mat4_cast(m_orientation);
}

Point Camera::worldToNdc(const Point &world) const
{
    auto viewProj = viewProjMatrix();

    glm::dvec4 p = glm::dvec4((double)world.x(), (double)world.y(), (double)world.z(), 1.0);
    glm::dvec4 clipSpace = viewProj * p;
    glm::dvec3 ndc = glm::xyz(clipSpace) / clipSpace.w;

    return Point(ndc.x, ndc.y, ndc.z);
}

Point Camera::worldToView(const Point& world) const
{
    auto viewMat = viewMatrix();

    glm::dvec4 p = glm::dvec4((double)world.x(), (double)world.y(), (double)world.z(), 1.0);
    p = viewMat * p;
    glm::dvec3 view = glm::xyz(p / p.w); // Not really needed but whatever

    return Point(view.x, view.y, view.z);
}

Point Camera::viewToWorld(const Point& view) const
{
    auto cameraTransform = transform();

    glm::dvec4 p = glm::dvec4((double)view.x(), (double)view.y(), (double)view.z(), 1.0);
    p = cameraTransform * p;
    glm::dvec3 world = glm::xyz(p / p.w); // Not really needed but whatever

    return Point(world.x, world.y, world.z);
}

Ray Camera::ndcToWorldRay(const Point& ndc) const
{
    auto cameraTransform = transform();

    auto rayView = m_projection->ndcToViewRay(ndc);

    glm::dvec3 originCam = glm::dvec3(rayView.origin.x(), rayView.origin.y(), rayView.origin.z());
    glm::dvec3 dirCam = glm::dvec3(rayView.direction.x(), rayView.direction.y(), rayView.direction.z());

    // Rotate the direction (w = 0) to world space and normalize
    glm::dvec3 rayOrigin = glm::xyz(cameraTransform * glm::dvec4(originCam, 1.0));
    glm::dvec3 rayDirWorld = glm::normalize(glm::xyz(cameraTransform * glm::dvec4(dirCam, 0.0)));
    
    Ray rayWorld;
    rayWorld.origin = {rayOrigin.x, rayOrigin.y, rayOrigin.z};
    rayWorld.direction = {rayDirWorld.x, rayDirWorld.y, rayDirWorld.z};

    return rayWorld;
}

double CameraProjection::unitsPerPixelAtDistanceNumerical(double zDistCamera) const
{
    // TODO
    // auto p1 = ndcToViewRay({-1, 0, 0});
    // auto p2 = ndcToViewRay({1, 0, 0});
    // p1 = p1 * (zDistCamera / p1.z());
    // p2 = p2 * (zDistCamera / p2.z());
    // double widthCam = (p1-p2).length3D();

    // return widthCam / width();
    return 1.0;
}
