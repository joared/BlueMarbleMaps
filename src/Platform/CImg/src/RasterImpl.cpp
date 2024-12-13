#include "Raster.h"
#include "CImg.h"

using namespace BlueMarble;

void takeCImgData(cimg_library::CImg<unsigned char>& cimg, unsigned char** data, int& width, int& height, int& channels)
{
    // width = cimg.width();
    // height = cimg.height();
    // channels = cimg.spectrum();
    // *data = cimg._data;
    //cimg._data = nullptr;
}

class Raster::Impl
{
    public:
        Impl()
            : m_img(1, 1, 1, 3, 0)
        {
            
            std::cout << "Raster::Raster() Warning: Raster not initialized\n";
        }

        Impl(const Raster &raster)
            : m_img(raster.m_impl->m_img)
        {
        }

        Impl(int width, int height, int channels, int fill)
            : m_img(width, height, 1, channels, fill)
        {
        }

        Impl(unsigned char* data, int width, int height, int channels)
            : m_img(data, width, height, 1, channels, false)
        {
        }

        Impl(const std::string& filePath)
            : m_img(cimg_library::CImg<unsigned char>(filePath.c_str()))
        {
        }


        int width() const
        {
            return m_img.width();
        }


        int height() const
        {
            return m_img.height();
        }

        int channels() const
        {
            return m_img.spectrum();
        }

        void resize(int width, int height, ResizeInterpolation interpolation)
        {
            int interpolationType = (int)interpolation;
            m_img.resize(width, height, -100, -100, interpolationType);
            
        }
        void resize(float scaleRatio, ResizeInterpolation interpolation)
        {
            int interpolationType = (int)interpolation;
            // interpolationType = interpolationType == 0 ? -1 : interpolationType; // -1 does not work as I expect
            m_img.resize(width()*scaleRatio, height()*scaleRatio, -100, -100, interpolationType);
        }

        void rotate(double angle, int cx, int cy, ResizeInterpolation interpolation)
        {
            int interpolationType = (int)interpolation;
            m_img.rotate(angle, cx, cy, interpolationType, 0);
        }

        void fill(int val)
        {
            m_img.fill(val);
        }

        void blur(double sigmaX, double sigmaY, double sigmaZ, bool isGaussian)
        {
            m_img.blur(sigmaX, sigmaY, sigmaZ, isGaussian);
        }

        Raster getCrop(int x0, int y0, int x1, int y1)
        {
            auto raster = Raster(0,0,0,0); // prevent warning
            raster.m_impl->m_img = m_img.get_crop(x0, y0, x1, y1);

            return raster;
        }

        const unsigned char* data() const
        {
            return m_img.data();
        }

        void operator=(const Raster &raster)
        {
            m_img = raster.m_impl->m_img;
        }

    private:
        cimg_library::CImg<unsigned char> m_img; // TODO: remove this dependency when implementing pimpl
};