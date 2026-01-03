#include "BlueMarbleMaps/Core/Camera/Camera.h"


using namespace BlueMarble;

Point CameraProjection::viewToNdc(const Point &view) const
{
    auto proj = projectionMatrix();

    glm::vec4 p = glm::vec4((float)view.x(), (float)view.y(), (float)view.z(), 1.0f);
    glm::vec4 clipSpace = proj * p;
    glm::vec3 ndc = glm::xyz(clipSpace) / clipSpace.w;

    return Point(ndc.x, ndc.y, ndc.z);
}

Point CameraProjection::ndcToView(const Point& ndc) const
{
    auto proj = projectionMatrix();

    glm::vec4 ndcGlm(ndc.x(), ndc.y(), ndc.z(), 1.0f);
    glm::vec4 homo = glm::inverse(proj) * ndcGlm;
    glm::vec3 view = glm::xyz(homo / homo.w);

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

Point Camera::worldToNdc(const Point& world) const
{
    auto viewProj = viewProjMatrix();

    glm::vec4 p = glm::vec4((float)world.x(), (float)world.y(), (float)world.z(), 1.0f);
    glm::vec4 clipSpace = viewProj * p;
    glm::vec3 ndc = glm::xyz(clipSpace) / clipSpace.w;

    return Point(ndc.x, ndc.y, ndc.z);
}

Point Camera::worldToView(const Point& world) const
{
    auto viewMat = viewMatrix();

    glm::vec4 p = glm::vec4((float)world.x(), (float)world.y(), (float)world.z(), 1.0f);
    p = viewMat * p;
    glm::vec3 view = glm::xyz(p / p.w); // Not really needed but whatever

    return Point(view.x, view.y, view.z);
}

Point Camera::viewToWorld(const Point& view) const
{
    auto cameraTransform = transform();

    glm::vec4 p = glm::vec4((float)view.x(), (float)view.y(), (float)view.z(), 1.0f);
    p = cameraTransform * p;
    glm::vec3 world = glm::xyz(p / p.w); // Not really needed but whatever

    return Point(world.x, world.y, world.z);
}

Ray Camera::ndcToWorldRay(const Point& ndc) const
{
    auto cameraTransform = transform();

    auto rayView = m_projection->ndcToViewRay(ndc);

    glm::vec3 originCam = glm::vec3(rayView.origin.x(), rayView.origin.y(), rayView.origin.z());
    glm::vec3 dirCam = glm::vec3(rayView.direction.x(), rayView.direction.y(), rayView.direction.z());

    // Rotate the direction (w = 0) to world space and normalize
    glm::vec3 rayOrigin = glm::xyz(cameraTransform * glm::vec4(originCam, 1.0));
    glm::vec3 rayDirWorld = glm::normalize(glm::xyz(cameraTransform * glm::vec4(dirCam, 0.0)));
    
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
