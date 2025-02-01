#include "BlueMarbleMaps/CoordinateSystem/GeodeticDatum.h"

using namespace BlueMarble;

GeodeticDatumPtr GeodeticDatum::wgs84()
{
    return std::make_shared<GeodeticDatum>
    (
        Ellipsoid::wgs84Spheroid(),
        PrimeMeridian::greenwhich(),
        nullptr // TODO: IdentityDatumShift(GeodeticDatum::wgs84())
    );
}


EllipsoidPtr Ellipsoid::spheroid(double semiMajorAxisLength, double flatteningFactor)
{
    // semi-minor axis
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


PrimeMeridianPtr PrimeMeridian::greenwhich()
{
    return PrimeMeridianPtr(new PrimeMeridian(0.0));
}
