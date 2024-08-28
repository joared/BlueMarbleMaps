#ifndef ANIMATION
#define ANIMATION

#include "Core.h"
#include <memory>

namespace BlueMarble
{
    class Map; // Forward declaration

    struct InertiaOptions
    {
        double alpha = 0.001;
        double linearity = 0.2;
    };

    class Animation; // Forward declaration
    typedef std::shared_ptr<Animation> AnimationPtr;

    class Animation
    {
        public:
            static AnimationPtr Create(Map& map, const Point& from, const Point& to, double duration, bool isInertial, const InertiaOptions& options = InertiaOptions());
            static AnimationPtr Create(Map& map, const Point& from, const Point& to, double fromScale, double toScale, double duration, bool isInertial, const InertiaOptions& options = InertiaOptions());
            Animation(Map& map, const Point& from, const Point& to, double duration, double fromScale, double toScale, bool isInertial, const InertiaOptions& options = InertiaOptions());

            const Point& fromCenter() { return m_from; }
            const Point& toCenter() { return m_to; }
            double fromScale() { return m_fromScale; }
            double toScale() { return m_toScale; }

            bool update(double elapsed);
        private:
            double easeOut(double ratio, double easeOutPower);

            Map&  m_map;
            Point m_from;
            Point m_to;
            double m_fromScale;
            double m_toScale;
            double m_duration;
            bool m_isInertial;
            const InertiaOptions& m_options;
    };

}
#endif /* ANIMATION */
