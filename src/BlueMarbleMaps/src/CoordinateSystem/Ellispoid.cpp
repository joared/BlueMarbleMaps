#include "BlueMarbleMaps/CoordinateSystem/Ellipsoid.h"

using namespace BlueMarble;

EllipsoidPtr Ellipsoid::spheroid(double semiMajorAxisLength, double flatteningFactor)
{
    double semiMinor = semiMajorAxisLength*(1.0-flatteningFactor);
    return std::make_shared<Ellipsoid>(semiMajorAxisLength, 
                                       semiMajorAxisLength, 
                                       semiMinor);
}


EllipsoidPtr Ellipsoid::wgs84Spheroid()
{
    return spheroid(6378137.0, 1.0 / 298.257223563);
}


double Ellipsoid::inverseFlattening() const
{
    return 1.0 / (1.0 - m_c / m_a);
}