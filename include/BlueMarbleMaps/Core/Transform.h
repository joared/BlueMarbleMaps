#ifndef BLUEMARBLE_TRANSFORM
#define BLUEMARBLE_TRANSFORM

#include "BlueMarbleMaps/Core/Core.h"

namespace BlueMarble
{
    class Transform
    {
        public:
            static inline Transform screenTransform(int width, int height)
            {
                auto center = Point((width-1)*0.5, (height-1)*0.5);
                return Transform(center, 1.0, -1.0, 0.0);
            }

            inline Transform() 
                : m_translation(Point(0,0))
                , m_scaleX(1.0)
                , m_scaleY(1.0)
                , m_scaleXY(1.0)
                , m_rotation(0.0)
            {
            }

            inline Transform(const Point& translation, double scale, double rotation) 
                : m_translation(translation)
                , m_scaleX(scale)
                , m_scaleY(scale)
                , m_scaleXY(scale)
                , m_rotation(rotation)
            {
            }

            inline Transform(const Point& translation, double scaleX, double scaleY, double rotation) 
                : m_translation(translation)
                , m_scaleX(scaleX)
                , m_scaleY(scaleY)
                , m_scaleXY(scaleX*scaleY*0.5) // Not sure
                , m_rotation(rotation)
            {
            }

            inline const Point& translation() const { return m_translation; }
            inline double scaleX() const { return m_scaleX; }
            inline double scaleY() const { return m_scaleY; }
            inline double scaleXY() const { return m_scaleXY; }
            inline double rotation() const { return m_rotation; }

        private:
            Point m_translation;
            double m_scaleX;
            double m_scaleY;
            double m_scaleXY;
            double m_rotation;
    };
}

#endif /* BLUEMARBLE_TRANSFORM */
