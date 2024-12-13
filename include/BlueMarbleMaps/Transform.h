#ifndef BLUEMARBLE_TRANSFORM
#define BLUEMARBLE_TRANSFORM

#include "Core.h"

namespace BlueMarble
{
    class Transform
    {
        public:
            inline Transform() 
                : m_translation(Point(0,0))
                , m_scale(1.0)
                , m_rotation(0.0)
            {
            }

            inline Transform(const Point& translation, double scale, double rotation) 
                : m_translation(translation)
                , m_scale(scale)
                , m_rotation(rotation)
            {
            }

            inline const Point& translation() { return m_translation; }
            inline double scale() { return m_scale; }
            inline double rotation() { return m_rotation; }

        private:
            Point m_translation;
            double m_scale;
            double m_rotation;
    };
}

#endif /* BLUEMARBLE_TRANSFORM */
