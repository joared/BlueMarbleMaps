#ifndef BLUEMARBLE_PROJECTION
#define BLUEMARBLE_PROJECTION

#include <memory>

namespace BlueMarble
{
    // Forward declarations and typedefs
    class Projection; typedef std::shared_ptr<Projection> ProjectionPtr;
    class MercatorProjection; typedef std::shared_ptr<MercatorProjection> MercatorProjectionPtr;
    class MercatorProjection; typedef std::shared_ptr<MercatorProjection> MercatorProjectionPtr;
    class StereoGraphicProjection; typedef std::shared_ptr<StereoGraphicProjection> StereoGraphicProjectionPtr;
    class LongLatProjection; typedef std::shared_ptr<LongLatProjection> LongLatProjectionPtr;

    class Projection
    {
        public:
            Projection();
    };

    class MercatorProjection : public Projection
    {

    };

    class StereoGraphicProjection : public Projection
    {

    };

    class LongLatProjection : public Projection
    {

    };

} // namespace BlueMarble


#endif /* BLUEMARBLE_PROJECTION */

