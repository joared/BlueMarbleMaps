#include "BlueMarbleMaps/Core/Camera.h"

using namespace BlueMarble;

Point CameraProjection::viewToNdc(const Point &view) const
{
    return Point();
}

Point CameraProjection::ndcToView(const Point &view) const
{
    return Point();
}

Point Camera::worldToNdc(const Point& world) const
{
    glm::vec4 p = glm::vec4((float)world.x(), (float)world.y(), (float)world.z(), 1.0f);
    glm::vec4 clipSpace = viewProjMatrix() * p;
    glm::vec3 ndc = glm::xyz(clipSpace) / clipSpace.w;

    return Point(ndc.x, ndc.y, ndc.z);
}

Point Camera::ndcToWorldRay(const Point& ndc) const
{
    glm::vec4 nearPointNdc(ndc.x(), ndc.y(), ndc.z(), 1.0f);
    glm::vec4 nearCameraAffine = glm::inverse(projectionMatrix()) * nearPointNdc;
    glm::vec3 nearCamera = glm::xyz(nearCameraAffine / nearCameraAffine.w);

    // Ray vector in camera space (we don't need to normalize yet)
    glm::vec3 dirCamNotNormalized = nearCamera;

    // Rotate the direction (w = 0) to world space and normalize
    glm::vec3 rayDirWorld = glm::normalize(glm::xyz(transform() * glm::vec4(dirCamNotNormalized, 0.0)));

    return Point(rayDirWorld.x, rayDirWorld.y, rayDirWorld.z);
}

double CameraProjection::unitsPerPixelAtDistanceNumerical(double zDistCamera) const
{
    // TODO
    // int w = m_projection->width();
    // auto p1 = rayDirectionCamera(0,0);
    // auto p2 = rayDirectionCamera(w,0);
    // p1 = p1 * (zDistCamera / p1.z());
    // p2 = p2 * (zDistCamera / p2.z());
    // double widthCam = (p1-p2).length3D();

    // return w / widthCam;
    return 1.0; 
}
