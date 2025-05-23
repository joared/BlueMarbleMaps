#ifndef BLUEMARBLE_COLOR
#define BLUEMARBLE_COLOR

#include "BlueMarbleMaps/Utility/Utils.h"
#include <string>

namespace BlueMarble
{
    class Color
    {
        public:
            inline static Color transparent() { return Color(0, 0, 0, 0); }
            inline static Color red(double a=1.0) { return Color(255, 0, 0, a); }
            inline static Color green(double a=1.0) { return Color(0, 255, 0, a); }
            inline static Color blue(double a=1.0) { return Color(0, 0, 255, a); }
            inline static Color white(double a=1.0) { return Color(255, 255, 255, a); }
            inline static Color black(double a=1.0) { return Color(0, 0, 0, a); }

            // inline Color(int r, int g, int b)
            //     : m_r(r)
            //     , m_g(g)
            //     , m_b(b)
            //     , m_alpha(1.0)
            // {}

            inline Color(int r=0, int g=0, int b=0, double alpha=1.0)
                : m_r(Utils::clampValue(r, 0, 255))
                , m_g(Utils::clampValue(g, 0, 255))
                , m_b(Utils::clampValue(b, 0, 255))
                , m_alpha(Utils::clampValue(alpha, 0, 1.0))
            {}

            inline int r() const { return m_r; }
            inline int b() const { return m_b; }
            inline int g() const { return m_g; }
            inline double a() const { return m_alpha; }

            inline std::string toString() const 
            { 
                std::string str = "Color (";
                str += "r:" + std::to_string(r());
                str += ", g:" + std::to_string(g());
                str += ", b:" + std::to_string(b());
                str += ", alpha:" + std::to_string(a());
                str += ")";

                return str;
            }

        private:
            int m_r;
            int m_g;
            int m_b;
            double m_alpha;
    };
}

#endif /* BLUEMARBLE_COLOR */
