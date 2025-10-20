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
    class Crs
    {
        public:
            static CrsPtr wgs84LngLat();
            Crs(const GeodeticDatumPtr& datum, const ProjectionPtr& projection);
            Point projectTo(const CrsPtr& crs, const Point& point);
            Rectangle projectTo(const CrsPtr& crs, const Rectangle& rect);
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
            const GeodeticDatumPtr& datum() { return m_datum; };
            const ProjectionPtr& projection() { return m_projection; }
        private:
            GeodeticDatumPtr m_datum;
            ProjectionPtr    m_projection;
    };

} // namespace BlueMarble

#endif /* BLUEMARBLE_CRS */

