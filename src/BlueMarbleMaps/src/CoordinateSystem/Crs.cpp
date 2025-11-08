#include "BlueMarbleMaps/CoordinateSystem/Crs.h"


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

Crs::Crs(const GeodeticDatumPtr& datum, const ProjectionPtr& projection)
    : m_datum(datum)
    , m_projection(projection)
    , m_id(-1) // Invalid id
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
    //auto lngLatBounds = Rectangle(-179.99, -89.99, 179.99, 89.99);
    auto lngLatBounds = Rectangle(-180.0, -85.05112878, 180.0, 85.05112878);
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

double Crs::globalMeterScale() const
{
    return m_projection->globalMeterScale(m_datum->ellipsoid());
}

double Crs::localMeterScaleAt(const Point& point) const
{
    auto lngLat = m_projection->unProject(point, m_datum->ellipsoid());
    return m_projection->localMeterScaleAt(lngLat, m_datum->ellipsoid());
}
