#ifndef BLUEMARBLE_EFFECT
#define BLUEMARBLE_EFFECT

#include "Raster.h"
#include "Drawable.h"
#include <memory>

namespace BlueMarble
{

    class IEffect
    {
        public:
            virtual ~IEffect() = default;
            virtual void apply(Drawable& drawable, Raster& raster) = 0;
    };
    typedef std::shared_ptr<IEffect> EffectPtr;

    class DropShadowEffect : public IEffect
    {
        public:
            DropShadowEffect(double blurRadius=3, int offsetX=10, int offsetY=10, double strength=2.0);
            void apply(Drawable& drawable, Raster& raster);
        private:
            double  m_blurRadius;
            int     m_offsetX;
            int     m_offsetY;
            double  m_strength;
    };
    typedef std::shared_ptr<DropShadowEffect> DropShadowEffectPtr;

}

#endif /* BLUEMARBLE_EFFECT */
