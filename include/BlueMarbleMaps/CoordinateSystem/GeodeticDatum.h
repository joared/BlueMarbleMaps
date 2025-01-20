#ifndef BLUEMARBLE_GEODETICDATUM
#define BLUEMARBLE_GEODETICDATUM

#include <cstdint>
#include <memory>

namespace BlueMarble
{

    // Forward declarations and typedefs
    class Ellipsoid; typedef std::shared_ptr<Ellipsoid> EllipsoidPtr;
    class PrimeMeridian; typedef std::shared_ptr<PrimeMeridian> PrimeMeridianPtr;
    class DatumShift; typedef std::shared_ptr<DatumShift> DatumShiftPtr;
    class GeodeticDatum; typedef std::shared_ptr<GeodeticDatum> GeodeticDatumPtr;

    class Ellipsoid
    {
        public:
            static EllipsoidPtr spheroid(double semiMajorAxisLength, double flatteningFactor);
            static EllipsoidPtr wgs84Spheroid();
            Ellipsoid(double a, double b, double c)
                : m_a(a)
                , m_b(b)
                , m_c(c)
            {}

            double a() const { return m_a; }
            double b() const { return m_b; }
            double c() const { return m_c; }
            double inverseFlattening() const;

        private:
            double m_a; // X-axis length
            double m_b; // Y-axis length
            double m_c; // Z-axis length
    };

    class PrimeMeridian
    {
        public:
            static PrimeMeridianPtr greenwhich();

            double longitudeFromGreenwhich() const { return m_longiTudeFromGreenwhich; };
        private:
            PrimeMeridian(double longitudeFromGreenwhich)
                : m_longiTudeFromGreenwhich(longitudeFromGreenwhich)
            {}

            double m_longiTudeFromGreenwhich;
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

