#include <CImg.h>
using namespace cimg_library;

int main() {
        // Load the original image (ensure it's RGBA if it has transparency)
    CImg<unsigned char> img("/home/joar/BlueMarbleMaps/geodata/symbols/airplane.png");

    const unsigned char purple[] = {128, 0, 128, 100};
    img.draw_circle(50, 50, 100, purple, 1);
    img.display("Image");

    // Check if the image has an alpha channel
    if (img.spectrum() != 4) {
        std::cerr << "The image does not have an alpha channel! Ensure it's RGBA.\n";
        return -1;
    }

    std::cout << "Depth: " << img.depth() << "\n";
    

    auto background = CImg<unsigned char>(img.width(), img.height(), 1, 3, 200);
    

    // Shadow parameters
    const int shadow_offset_x = 100;  // Horizontal shadow offset
    const int shadow_offset_y = 100;  // Vertical shadow offset
    const float shadow_blur_radius = 15.0f;  // Shadow softness

    // Create a shadow image from the alpha channel of the PNG
    cimg_library::CImg<unsigned char> shadowAlpha(img.get_channel(3));
    shadowAlpha.blur(shadow_blur_radius);  // Blur for soft shadow
    shadowAlpha.display("Shadow mask");
    auto shadow = CImg<unsigned char>(img.width(), img.height(), 1, 3, 0);

    // Apply the shadow with an offset on the background
    background.draw_image(shadow_offset_x, shadow_offset_y, shadow, shadowAlpha, 1, 255);
    background.display("Background with shadow");

    // Draw the PNG image over the background, respecting transparency
    background.draw_image(0, 0, img, img.get_shared_channel(3), 1, 255);

    // Save or display the result
    background.save("output_with_shadow.png");
    background.display("Final Output");

    return 0;
    // // Load the original image (ensure it's RGBA if it has transparency)
    // CImg<unsigned char> img("/home/joar/BlueMarbleMaps/geodata/symbols/aeroplane.png");


    // // Check if the image has an alpha channel
    // if (img.spectrum() != 4) {
    //     std::cerr << "The image does not have an alpha channel! Ensure it's RGBA.\n";
    //     return -1;
    // }

    // std::cout << "Depth: " << img.depth() << "\n";
    

    // auto background = CImg<unsigned char>(img.width(), img.height(), 1, 3, 200);
    // //const unsigned char purple[] = {128, 0, 128};
    // background.display("Background");

    // // Shadow parameters
    // const int shadow_offset_x = 100;  // Horizontal shadow offset
    // const int shadow_offset_y = 100;  // Vertical shadow offset
    // const float shadow_blur_radius = 15.0f;  // Shadow softness

    // // Create a shadow image from the alpha channel of the PNG
    // auto shadowAlpha = CImg<unsigned char>(img.width(), img.height(), 1, 1, 0);
    // shadowAlpha.draw_image(shadow_offset_x, shadow_offset_y, img.get_channel(3));  // Extract alpha channel
    // shadowAlpha.blur(shadow_blur_radius);  // Blur for soft shadow
    // shadowAlpha.display("Shadow mask");

    // auto shadow = CImg<unsigned char>(img.width(), img.height(), 1, 3, 0);

    // // Apply the shadow with an offset on the background
    // background.draw_image(0, 0, shadow, shadowAlpha, 1, 255);
    // background.display("Background with shadow");

    // // Draw the PNG image over the background, respecting transparency
    // background.draw_image(0, 0, img, img.get_shared_channel(3), 1, 255);

    // // Save or display the result
    // background.save("output_with_shadow.png");
    // background.display("Final Output");

    // return 0;
}