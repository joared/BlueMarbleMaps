#ifndef BLUEMARBLE_ANIMATIONFUNCTIONS
#define BLUEMARBLE_ANIMATIONFUNCTIONS

#include "BlueMarbleMaps/Core/Core.h"

namespace BlueMarble
{
    namespace AnimationFunctions
    {
        
        constexpr double inverseProgressAt(double progress, double at)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);
            assert(at >= 0.0);
            assert(at <= 1.0);

            if (progress >= at)
                 return 1.0 - progress;
            
            return progress;
        }

        constexpr double inverseProgress(double progress)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);

            return 1.0 - progress;
        }

        constexpr double offsetProgress(double progress, double offset)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);

            double p = progress+offset;
            if (p > 1.0) p = p - int(p);

            return p;
        }

        constexpr double subDivideProgress(double progress, int nCycles)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);
            assert(nCycles > 0);
            double p = progress*nCycles;
            p = p - int(p);

            return p;
        }

        constexpr bool isAlternateProgressInReverse(double progress)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);
         
            return progress*2.0 > 1.0;
        }

        constexpr double alternateProgress(double progress)
        {
            assert(progress >= 0.0);
            assert(progress <= 1.0);
            
            double p = progress*2.0;
            if (p > 1.0)
                return 2.0 - p;

            return p;
        }

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

        using AnimFunc = std::function<double(double)>;
        class AnimationBuilder
        {
        public:
            AnimationBuilder()
                : m_animationChain()
            {
            }
            AnimationBuilder& subDivide(int nCycles)
            {
                extend([nCycles](double p){ return AnimationFunctions::subDivideProgress(p, nCycles); });
                return *this;
            };
            AnimationBuilder& alternate()
            {
                extend(AnimationFunctions::alternateProgress);
                return *this;
            };
            AnimationBuilder& inverse()
            {
                extend(AnimationFunctions::inverseProgress);
                return *this;
            };
            AnimationBuilder& inverseAt(double at)
            {
                extend([at](double p){ return AnimationFunctions::inverseProgressAt(p, at); });
                return *this;
            };
            AnimationBuilder& offset(double offset)
            {
                extend([offset](double p){ return AnimationFunctions::offsetProgress(p, offset); });
                return *this;
            };
            AnimationBuilder& easeInCubic()
            {
                extend(AnimationFunctions::easeInCubic);
                return *this;
            };
            AnimationBuilder& easeOut(double easeOutPower)
            {
                extend([easeOutPower](double p){ return AnimationFunctions::easeOut(p, easeOutPower); });
                return *this;
            };

            AnimFunc build() const
            { 
                return [funcs = m_animationChain](double p){
                    for (auto const& f : funcs)
                    {
                        p = f(p);
                    }
                    return p;
                };
            }
        private:
            void extend(AnimFunc nextFunc)
            {
                m_animationChain.emplace_back(std::move(nextFunc));
            }
            std::vector<AnimFunc> m_animationChain;

        };

    }
}

#endif /* BLUEMARBLE_ANIMATIONFUNCTIONS */
