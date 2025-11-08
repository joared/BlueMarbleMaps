#ifndef BLUEMARBLE_CRS
#define BLUEMARBLE_CRS

#include "BlueMarbleMaps/CoordinateSystem/GeodeticDatum.h"
#include "BlueMarbleMaps/CoordinateSystem/Projection.h"
#include "BlueMarbleMaps/Core/Core.h"

#include <memory>

namespace BlueMarble
{
    
    class Crs;
    typedef std::shared_ptr<Crs> CrsPtr;
    class Crs : public std::enable_shared_from_this<Crs>
    {
        public:
            static CrsPtr wgs84LngLat();
            static CrsPtr wgs84MercatorWeb();

            bool isFunctionallyEquivalent(const CrsPtr& otherCrs);
            int64_t id() const { return m_id; };
            Rectangle bounds();
            Point projectTo(const CrsPtr& crs, const Point& point) const;
            Rectangle projectTo(const CrsPtr& crs, const Rectangle& rect) const;
            template<typename Iter>
            PointCollectionPtr projectTo(const CrsPtr& crs, const Iter& pointsFirst, const Iter& pointsLast)
            {
                PointCollectionPtr points = std::make_shared<PointCollection>();

                for (auto it=pointsFirst; it!=pointsLast;++it)
                {
                    points->emplace(projectTo(crs, *it));
                }

                return points;
            }
            double globalMeterScale() const;
            double localMeterScaleAt(const Point& p) const;
            const GeodeticDatumPtr& datum() { return m_datum; };
            const ProjectionPtr& projection() { return m_projection; }
        private:
            // We currently only support a subset of coordinate systems exposed with static methods
            // so we prevent creation of custom coordinate systems by making the constructor private
            Crs(const GeodeticDatumPtr& datum, const ProjectionPtr& projection);
            void id(int64_t id) { m_id = id; };

            GeodeticDatumPtr m_datum;
            ProjectionPtr    m_projection;
            int64_t          m_id;
    };

} // namespace BlueMarble

#endif /* BLUEMARBLE_CRS */

