#ifndef BLUEMARBLE_CRS
#define BLUEMARBLE_CRS

#include "CoordinateSystem/GeodeticDatum.h"
#include "CoordinateSystem/Projection.h"
#include "Core/Core.h"

#include <memory>

namespace BlueMarble
{
    
    class Crs;
    typedef std::shared_ptr<Crs> CrsPtr;
    class Crs
    {
        public:
            static CrsPtr wgs84LngLat();
            Crs();
            void projectTo(const CrsPtr& crs, const Point& point);
            void projectTo(const CrsPtr& crs, const Rectangle& rect);
        private:
            GeodeticDatumPtr m_datum;
            ProjectionPtr    m_projection;
    };

} // namespace BlueMarble

#endif /* BLUEMARBLE_CRS */

