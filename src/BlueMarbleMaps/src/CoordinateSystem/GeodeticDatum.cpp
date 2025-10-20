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


PrimeMeridianPtr PrimeMeridian::greenwhich()
{
    return PrimeMeridianPtr(new PrimeMeridian(0.0));
}
