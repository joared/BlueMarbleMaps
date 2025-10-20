#ifndef BLUEMARBLE_GEODETICDATUM
#define BLUEMARBLE_GEODETICDATUM

#include "BlueMarbleMaps/CoordinateSystem/Ellipsoid.h"

#include <cstdint>
#include <memory>

namespace BlueMarble
{

    // Forward declarations and typedefs
    class PrimeMeridian; typedef std::shared_ptr<PrimeMeridian> PrimeMeridianPtr;
    class DatumShift; typedef std::shared_ptr<DatumShift> DatumShiftPtr;
    class GeodeticDatum; typedef std::shared_ptr<GeodeticDatum> GeodeticDatumPtr;

    class PrimeMeridian
    {
        public:
            static PrimeMeridianPtr greenwhich();

            double longitudeFromGreenwhich() const { return m_longitudeFromGreenwhich; };
        private:
            PrimeMeridian(double longitudeFromGreenwhich)
                : m_longitudeFromGreenwhich(longitudeFromGreenwhich)
            {}

            double m_longitudeFromGreenwhich;
    };

    class DatumShift
    {
        // TODO
        // Shift relative some well known datum
        public:
            const GeodeticDatumPtr& knownDatum() const { return m_knownDatum; } 
        protected:
            DatumShift(const GeodeticDatumPtr& knownDatum)
                : m_knownDatum(knownDatum)
            {}
        private:
            GeodeticDatumPtr m_knownDatum;
    };

    class IdentityDatumShift : public DatumShift
    {
        public:
            IdentityDatumShift(const GeodeticDatumPtr& knownDatum)
                : DatumShift(knownDatum)
            {}
    };

    class GeodeticDatum
    {
        public:
            static GeodeticDatumPtr wgs84();
            GeodeticDatum(const EllipsoidPtr& ellipsoid, 
                          const PrimeMeridianPtr& primeMeridian, 
                          const DatumShiftPtr& datumShift)
                : m_ellipsoid(ellipsoid)
                , m_primeMeridian(primeMeridian)
                , m_datumShift(datumShift)
            {}

            int64_t id() const { return m_id; };
            const EllipsoidPtr& ellipsoid() { return m_ellipsoid; }
            const PrimeMeridianPtr& primeMeridian() { return m_primeMeridian; }
            const DatumShiftPtr& datumShift() { return m_datumShift; }
        private:
            int64_t            m_id;
            EllipsoidPtr       m_ellipsoid;
            PrimeMeridianPtr   m_primeMeridian;
            DatumShiftPtr      m_datumShift;       // Shift relative some well known datum

    };

} // namespace BlueMarble


#endif /* BLUEMARBLE_GEODETICDATUM */

