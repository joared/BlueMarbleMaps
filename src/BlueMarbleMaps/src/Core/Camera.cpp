#include "BlueMarbleMaps/Core/Camera.h"


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
    glm::vec4 affine = glm::inverse(proj) * ndcGlm;
    glm::vec3 view = glm::xyz(affine / affine.w);

    return Point(view.x, view.y, view.z);
}

Point CameraProjection::ndcToViewRay(const Point& ndc) const
{
    // Ignore the z value and choose the point on the near plane (-1.0)
    return ndcToView({ndc.x(), ndc.y(), -1.0}).norm3D();
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

Point Camera::ndcToWorldRay(const Point& ndc) const
{
    auto proj = projectionMatrix();
    auto cameraTransform = transform();
    // Ignore the z value and choose the point on the near plane (-1.0)
    glm::vec4 nearPointNdc(ndc.x(), ndc.y(), -1.0f, 1.0f);
    glm::vec4 nearCameraAffine = glm::inverse(proj) * nearPointNdc;
    glm::vec3 nearCamera = glm::xyz(nearCameraAffine / nearCameraAffine.w);

    // Ray vector in camera space (we don't need to normalize yet)
    glm::vec3 dirCamNotNormalized = nearCamera;

    // Rotate the direction (w = 0) to world space and normalize
    glm::vec3 rayDirWorld = glm::normalize(glm::xyz(cameraTransform * glm::vec4(dirCamNotNormalized, 0.0)));

    return Point(rayDirWorld.x, rayDirWorld.y, rayDirWorld.z);
}

double CameraProjection::unitsPerPixelAtDistanceNumerical(double zDistCamera) const
{
    auto p1 = ndcToViewRay({-1, 0, 0});
    auto p2 = ndcToViewRay({1, 0, 0});
    p1 = p1 * (zDistCamera / p1.z());
    p2 = p2 * (zDistCamera / p2.z());
    double widthCam = (p1-p2).length3D();

    return widthCam / width();
}
