#include "BlueMarbleMaps/CoordinateSystem/Crs.h"
#include <ogr_spatialref.h>

using namespace BlueMarble;

// TODO: We only support these coordinate systems for now
// This is a temporary solution identifying the two
static constexpr int64_t wgs84LongLatId = 1;
static constexpr int64_t meractoWebId = 2;

CrsPtr Crs::wgs84LngLat()
{
    CrsPtr crs = std::shared_ptr<Crs>(new Crs(GeodeticDatum::wgs84(),
                                              Projection::longLat()));
    crs->id(wgs84LongLatId);

    return crs;
}

CrsPtr Crs::wgs84MercatorWeb()
{
    static constexpr int wgs84LongLatId = 2;
    CrsPtr crs = std::shared_ptr<Crs>(new Crs(GeodeticDatum::wgs84(),
                                              std::make_shared<MercatorWebProjection>()));
    crs->id(meractoWebId);

    return crs;
}

CrsPtr Crs::fromWkt(const std::string &wkt)
{
    OGRSpatialReference srs;
    const char* wktCStr = wkt.c_str();
    char* wktMutable = const_cast<char*>(wktCStr);
    if (srs.importFromWkt(&wktMutable) != OGRERR_NONE)
    {
        throw std::runtime_error("Invalid CRS WKT");
    }

    if (srs.IsGeocentric())
    {
        // TODO
        BMM_DEBUG() << "Geocentric\n";
        const char* auth = srs.GetAuthorityName("GEOGCCS");
        const char* code = srs.GetAuthorityCode("GEOGCCS");
        BMM_DEBUG() << "auth " << auth << "\n";
        BMM_DEBUG() << "code " << code << "\n";
    }

    if (srs.IsGeographic())
    {
        OGRSpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");

        if (srs.IsSame(&wgs84))
        {
            return Crs::wgs84LngLat();
        }
    }
    
    if (srs.IsProjected())
    {
        // --- Web Mercator ---
        OGRSpatialReference webMerc;
        webMerc.importFromEPSG(3857);

        if (srs.IsSame(&webMerc))
        {
            return Crs::wgs84MercatorWeb();
        }
    }

    // Unsupported CRS
    throw std::runtime_error("Unsupported CRS in dataset");
}

Crs::Crs(const GeodeticDatumPtr &datum, const ProjectionPtr &projection)
    : m_datum(datum), m_projection(projection), m_id(-1) // Invalid id
{
}

bool Crs::isFunctionallyEquivalent(const CrsPtr& otherCrs)
{
    assert(id() != -1);
    assert(otherCrs->id() != -1);

    // TODO: currently we use the id, but we can use parameters and stuff in the future
    return id() == otherCrs->id();
}

Rectangle Crs::bounds()
{
    auto lngLatBounds = Rectangle(-179.99, -89.99, 179.99, 89.99);
    //auto lngLatBounds = Rectangle(-180.0, -85.05112878, 180.0, 85.05112878); // Web mercator
    auto defaultCrs = Crs::wgs84LngLat();
    auto self = shared_from_this();
    return defaultCrs->projectTo(self, lngLatBounds);
}

Point Crs::projectTo(const CrsPtr& crs, const Point& point) const
{
    const auto& ellipsoid = m_datum->ellipsoid();
    auto lngLat = m_projection->unProject(point, ellipsoid);
    return crs->projection()->project(lngLat, ellipsoid);
}

Rectangle Crs::projectTo(const CrsPtr& crs, const Rectangle& rect) const
{
    std::vector<Point> newCorners;
    for (const auto& p : rect.corners())
    {
        newCorners.emplace_back(projectTo(crs, p));
    }

    return Rectangle::fromPoints(newCorners);
}

double Crs::globalMetersPerUnit() const
{
    return m_projection->globalMetersPerUnit(m_datum->ellipsoid());
}

double Crs::localMetersPerUnitAt(const Point& point) const
{
    auto lngLat = m_projection->unProject(point, m_datum->ellipsoid());
    return m_projection->localMetersPerUnitAt(lngLat, m_datum->ellipsoid());
}
