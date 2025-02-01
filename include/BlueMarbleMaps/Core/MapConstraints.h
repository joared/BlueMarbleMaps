#ifndef MAPCONSTRAINTS
#define MAPCONSTRAINTS

#include "BlueMarbleMaps/Core/Core.h"

namespace BlueMarble
{
    class Map; // Forward declaration

    class MapConstraints
    {
        public:
            MapConstraints(double maxScale, double minScale, const Rectangle& bounds)
                : m_maxScale(maxScale)
                , m_minScale(minScale)
                , m_bounds(bounds)
            {}

            void constrainMap(Map& map);
            double constrainValue(double val, double minVal, double maxVal);

            double maxScale() const { return m_maxScale; };
            void maxScale(double scale) { m_maxScale = scale; }
            double minScale() const { return m_minScale; };
            void minScale(double scale) { m_minScale = scale; }
            Rectangle& bounds() { return m_bounds; }

        private:
            double m_maxScale;
            double m_minScale;
            Rectangle m_bounds;
    };

} /* MAPCONSTRAINTS */

#endif /* MAPCONSTRAINTS */
