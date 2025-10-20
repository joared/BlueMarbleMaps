#ifndef BLUEMARBLE_ELLIPSOID
#define BLUEMARBLE_ELLIPSOID

#include <memory>

namespace BlueMarble
{
    class Ellipsoid; 
    using EllipsoidPtr = std::shared_ptr<Ellipsoid>;

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
}

#endif /* BLUEMARBLE_ELLIPSOID */
