#ifndef BLUEMARBLE_PROJECTION
#define BLUEMARBLE_PROJECTION

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/CoordinateSystem/Ellipsoid.h"

#include <memory>

namespace BlueMarble
{
    // Forward declarations and typedefs
    class Projection; 
    using ProjectionPtr = std::shared_ptr<Projection>;
    class LongLatProjection; 
    using LongLatProjectionPtr = std::shared_ptr<LongLatProjection>;
    class MercatorProjection; 
    using MercatorProjectionPtr = std::shared_ptr<MercatorProjection>;

    class Projection
    {
    public:
        Projection();
        virtual ~Projection() = default;
        static ProjectionPtr longLat();
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) = 0;
        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) = 0;
    };

    class LongLatProjection : public Projection
    {
    public:
        LongLatProjection() {}
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final { return lngLat; };
        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) override final { return point; };
    };

    class MercatorProjection : public Projection
    {
    public:
        MercatorProjection() {}
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final
        {
            double R = ellipsoid->a(); // Earth radius in meters

            double lon = lngLat.x();
            double lat = lngLat.y();
            double x = R * lon * M_PI / 180.0;
            double y = R * log(tan(M_PI/4.0 + (lat * M_PI / 180.0) / 2.0));

            return Point(x,y);
        };

        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) override final
        {
            double R = ellipsoid->a();

            double x = point.x();
            double y = point.y();

            double lon = x / R * 180.0 / M_PI;
            double lat = 2 * atan(exp(y / R)) - M_PI/2;
            lat = lat * 180.0 / M_PI;

            return Point(lon, lat);
        };
    };

} // namespace BlueMarble


#endif /* BLUEMARBLE_PROJECTION */

