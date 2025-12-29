#include "BlueMarbleMaps/CoordinateSystem/SurfaceModel.h"

using namespace BlueMarble;

PlaneSurfaceModel::PlaneSurfaceModel(const Point &origin, const Point &normal)
    : m_origin(origin)
    , m_normal(normal.norm3D())
{
}

bool PlaneSurfaceModel::rayIntersection(const Point& rayOrigin, const Point& rayDirection, double normalOffset,
                                        Point& surfacePoint, Point& surfaceNormal) const
{
    double denom = m_normal.dotProduct(rayDirection);

    // Ray parallel to plane
    if (std::abs(denom) < 1e-6f)
        return false;

    double t = m_normal.dotProduct(m_origin + m_normal*normalOffset - rayOrigin) * (1.0 / denom);

    // Intersection behind the ray origin
    if (t < 0.0)
        return false;

    surfacePoint = rayOrigin + rayDirection * t;
    surfaceNormal = m_normal;

    return true;
}
