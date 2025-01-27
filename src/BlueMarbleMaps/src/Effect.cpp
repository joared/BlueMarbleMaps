#include "Core/Effect.h"
#include <CImg.h>

using namespace BlueMarble;

DropShadowEffect::DropShadowEffect(double blurRadius, int offsetX, int offsetY, double strength)
    : m_blurRadius(blurRadius)
    , m_offsetX(offsetX)
    , m_offsetY(offsetY)
    , m_strength(strength)
{
}

void DropShadowEffect::apply(Drawable& drawable)
{
    // auto& img = raster;

    // if (img.colorDepth() != 4) {
    //     std::cerr << "The image does not have an alpha channel! Ensure it's RGBA.\n";
    //     throw std::exception();
    // }

    // auto& background = *static_cast<cimg_library::CImg<unsigned char>*>(drawable.getRaster().data());
    
    // // Create a shadow image (initialize with black)
    // cimg_library::CImg<unsigned char> shadowAlpha(img.get_channel(3)*m_strength);  // Same size as input
    // if (m_blurRadius > 0)
    //     shadowAlpha.blur(m_blurRadius);
    // auto shadow = cimg_library::CImg<unsigned char>(img.width(), img.height(), 1, 3, 0);

    // // Create the output image, same size as original

    // // Draw shadow offset on output
    // background.draw_image(m_offsetX, m_offsetY, shadow, shadowAlpha, 1, 255);
    // background.draw_image(0, 0, img, img.get_shared_channel(3), 1, 255);

    /*auto& img = *static_cast<cimg_library::CImg<unsigned char>*>(raster.data());
    
    if (img.spectrum() != 4) {
        std::cerr << "The image does not have an alpha channel! Ensure it's RGBA.\n";
        throw std::exception();
    }

    auto& background = *static_cast<cimg_library::CImg<unsigned char>*>(drawable.getRaster().data());
    
    // Create a shadow image (initialize with black)
    cimg_library::CImg<unsigned char> shadowAlpha(img.get_channel(3)*m_strength);  // Same size as input
    if (m_blurRadius > 0)
        shadowAlpha.blur(m_blurRadius);
    auto shadow = cimg_library::CImg<unsigned char>(img.width(), img.height(), 1, 3, 0);

    // Create the output image, same size as original

    // Draw shadow offset on output
    background.draw_image(m_offsetX, m_offsetY, shadow, shadowAlpha, 1, 255);
    background.draw_image(0, 0, img, img.get_channel(3), 1, 255);*/
}
