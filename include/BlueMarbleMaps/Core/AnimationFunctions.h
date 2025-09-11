#ifndef BLUEMARBLE_ANIMATIONFUNCTIONS
#define BLUEMARBLE_ANIMATIONFUNCTIONS

#include "BlueMarbleMaps/Core/Core.h"

namespace BlueMarble
{
    namespace AnimationFunctions
    {
        
        double easeInCubic(double t) 
        {
            return t * t * t;
        }

        double easeOut(double t, double easeOutPower)
        {
            return 1 - std::pow(1 - t, easeOutPower);
        }

        Point cubicBezier(double t, const Point& p0, const Point& p1, const Point& p2, const Point& p3) 
        {
            double u = 1.0 - t;
            double tt = t * t;
            double uu = u * u;
            double uuu = uu * u;
            double ttt = tt * t;

            Point result = p0 * uuu;               // (1 - t)^3 * P0
            result = result + (p1 * (3 * uu * t)); // 3 * (1 - t)^2 * t * P1
            result = result + (p2 * (3 * u * tt)); // 3 * (1 - t) * t^2 * P2
            result = result + (p3 * ttt);          // t^3 * P3

            return result;
        }

    }
}

#endif /* BLUEMARBLE_ANIMATIONFUNCTIONS */
