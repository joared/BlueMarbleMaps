#ifndef BLUEMARBLE_GEODETICDATUM
#define BLUEMARBLE_GEODETICDATUM

namespace BlueMarble
{
    
    class Ellipsoid
    {

    };

    class PrimeMeridian
    {

    };

    class DatumShift
    {

    };

    class GeodeticDatum
    {
        public:
            GeodeticDatum();
        private:
            Ellipsoid       m_ellipsoid;
            PrimeMeridian   m_primeMeridian;
            DatumShift      m_datumShift;

    };

} // namespace BlueMarble


#endif /* BLUEMARBLE_GEODETICDATUM */

