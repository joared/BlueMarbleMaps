#ifndef BLUEMARBLE_COLOR
#define BLUEMARBLE_COLOR

#include "BlueMarbleMaps/Utility/Utils.h"
#include <string>

namespace BlueMarble
{
    class Color
    {
        public:
            inline static Color undefined() { auto c = Color(-1, -1, -1, -1); c.m_isDefined=false; return c; }
            inline static Color transparent() { return Color(0, 0, 0, 0); }
            inline static Color red(double a=1.0) { return Color(255, 0, 0, a); }
            inline static Color green(double a=1.0) { return Color(0, 255, 0, a); }
            inline static Color blue(double a=1.0) { return Color(0, 0, 255, a); }
            inline static Color white(double a=1.0) { return Color(255, 255, 255, a); }
            inline static Color black(double a=1.0) { return Color(0, 0, 0, a); }
            inline static Color gray(double a=1.0) { return Color(128, 128, 128, a); }

            inline static std::vector<Color> colorRamp(const Color& start, const Color& end, int steps)
            {
                std::vector<Color> ramp;
                ramp.reserve(steps);

                double diffR = end.r()-start.r();
                double diffG = end.g()-start.g();
                double diffB = end.b()-start.b();
                double diffA = end.a()-start.a();

                for (int i = 0; i < steps; i++) 
                {
                    float t = static_cast<float>(i) / (steps - 1); // goes from 0 to 1
                    
                    ramp.push_back(Color(
                        (int)(start.r() + diffR*t),
                        (int)(start.g() + diffG*t),
                        (int)(start.b() + diffB*t),
                        start.a() + diffA*t)
                    );
                }

                return ramp;
            }

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
                , m_isDefined(true)
            {}
            inline bool isDefined() const { return m_isDefined; }
            inline int r() const { return m_r; }
            inline int b() const { return m_b; }
            inline int g() const { return m_g; }
            inline double a() const { return m_alpha; }

            inline Color operator+(const Color& other) const
            {
                return Color(r()+other.r(),
                             g()+other.g(),
                             b()+other.b(),
                             a()+other.a());
            }

            inline Color operator-(const Color& other) const
            {
                return Color(r()-other.r(),
                             g()-other.g(),
                             b()-other.b(),
                             a()-other.a());
            }

            inline Color operator*(const Color& other) const
            {
                return Color(r()*other.r(),
                             g()*other.g(),
                             b()*other.b(),
                             a()*other.a());
            }

            inline Color operator*(double fraction) const
            {
                return Color(r()*fraction,
                             g()*fraction,
                             b()*fraction,
                             a()*fraction);
            }

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
            bool m_isDefined;
    };
}

#endif /* BLUEMARBLE_COLOR */
