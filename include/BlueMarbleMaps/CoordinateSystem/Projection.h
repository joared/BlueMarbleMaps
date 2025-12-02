#ifndef BLUEMARBLE_PROJECTION
#define BLUEMARBLE_PROJECTION

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/CoordinateSystem/Ellipsoid.h"
#include "BlueMarbleMaps/Utility/Utils.h"

#include <memory>

namespace BlueMarble
{
    // Forward declarations and typedefs
    class Projection; 
    using ProjectionPtr = std::shared_ptr<Projection>;
    class LongLatProjection; 
    using LongLatProjectionPtr = std::shared_ptr<LongLatProjection>;
    class MercatorWebProjection; 
    using MercatorWebProjectionPtr = std::shared_ptr<MercatorWebProjection>;

    class Projection
    {
    public:
        Projection();
        virtual ~Projection() = default;
        static ProjectionPtr longLat();
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) = 0;
        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) = 0;
        virtual double globalMeterScale(const EllipsoidPtr& ellipsoid) = 0;                      // meters per unit in the projection
        virtual double localMeterScaleAt(const Point& lngLat, const EllipsoidPtr& ellipsoid) = 0; // meters per unit in the projection at a specific point
    };

    class LongLatProjection : public Projection
    {
    public:
        LongLatProjection() {}
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final { return lngLat; };
        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) override final { return point; };
        virtual double globalMeterScale(const EllipsoidPtr& ellipsoid) override final
        {
            return BMM_PI / 180.0 * ellipsoid->a();
        };
        virtual double localMeterScaleAt(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final
        {
            double lat = lngLat.y() * BMM_PI / 180.0;
            return BMM_PI / 180.0 * ellipsoid->a() * cos(lat);
        };
    };

    class MercatorWebProjection : public Projection
    {
    public:
        MercatorWebProjection() {}
        virtual Point project(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final
        {
            constexpr double MAX_LAT_WEB_MERCATOR = 85.05112878;
            double R = ellipsoid->a(); // Earth radius in meters

            double lon = lngLat.x();
            double lat = Utils::clampValue(lngLat.y(), -MAX_LAT_WEB_MERCATOR, MAX_LAT_WEB_MERCATOR);
            double x = R * lon * BMM_PI / 180.0;
            double y = R * log(tan(BMM_PI /4.0 + (lat * BMM_PI / 180.0) / 2.0));

            return Point(x,y);
        };

        virtual Point unProject(const Point& point, const EllipsoidPtr& ellipsoid) override final
        {
            double R = ellipsoid->a();

            double x = point.x();
            double y = point.y();

            double lon = x / R * 180.0 / BMM_PI;
            double lat = 2 * atan(exp(y / R)) - BMM_PI/2;
            lat = lat * 180.0 / BMM_PI;

            return Point(lon, lat);
        };

        virtual double globalMeterScale(const EllipsoidPtr& ellipsoid) override final
        {
            return 1.0;
        };

        virtual double localMeterScaleAt(const Point& lngLat, const EllipsoidPtr& ellipsoid) override final
        {
            double lat = lngLat.y() * BMM_PI / 180.0;
            return 1.0 / cos(lat); // scale distortion increases with latitude
        };
    };

} // namespace BlueMarble


#endif /* BLUEMARBLE_PROJECTION */

