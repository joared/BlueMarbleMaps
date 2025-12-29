#ifndef BLUEMARBLE_WORLDSURFACEMODEL
#define BLUEMARBLE_WORLDSURFACEMODEL

#include "BlueMarbleMaps/Core/Core.h"

#include <memory>

namespace BlueMarble
{
    class SurfaceModel
    {
        public:

            virtual ~SurfaceModel() = default;

            virtual bool rayIntersection(const Point& rayOrigin,
                                         const Point& rayDirection,
                                         double normalOffset,
                                         Point& surfacePoint,
                                         Point& surfaceNormal) const = 0;
    };
    using SurfaceModelPtr = std::shared_ptr<SurfaceModel>;

    class PlaneSurfaceModel : public SurfaceModel
    {
        public:
            PlaneSurfaceModel(const Point& origin, const Point& normal);

            bool rayIntersection(const Point& rayOrigin, 
                                 const Point& rayDirection,
                                 double normalOffset,
                                 Point& surfacePoint, 
                                 Point& surfaceNormal) const override final;
        
        private:
            Point m_origin;
            Point m_normal;
    };
    using PlaneSurfaceModelPtr = std::shared_ptr<PlaneSurfaceModel>;
}

#endif /* BLUEMARBLE_WORLDSURFACEMODEL */
