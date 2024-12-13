#ifndef BLUEMARBLE_RENDERER
#define BLUEMARBLE_RENDERER

#include "Core.h"
#include "Utils.h"
#include "Raster.h"

#include <memory>

namespace BlueMarble
{

    enum class RendererImplementation
    {
        Software,
        OpenGl
    };

    // TODO
    class Renderer
    {
        public:
            // virtual void drawCircle() = 0;
            // virtual void drawRect() = 0;
            // virtual void drawLine() = 0;
            // virtual void drawPolygon() = 0;
            // virtual void drawRaster() = 0;
            // virtual void drawText() = 0;
    };
    typedef std::shared_ptr<Renderer> RendererPtr;
};

#endif /* BLUEMARBLE_RENDERER */
