#include "OpenGLTexture.h"
#include <CImg.h>
#include <cassert>
#include <vector>

using namespace BlueMarble;

// OpenGLTexture::OpenGLTexture()
// 		: m_Width(m_Specification.Width), m_Height(m_Specification.Height)
// 	{
// 		m_InternalFormat = GL_RGBA;
// 		m_DataFormat = GL_RGBA;

// 		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
// 		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

// 		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 	}

void convertCImgToOpenGL(cimg_library::CImg<unsigned char>& cimgImage, 
                         unsigned char* openglData, 
                         int width, int height, int channels) 
{
    cimgImage.mirror('y');
    for (int c = 0; c < channels; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Compute OpenGL index (row-major order)
                int openglIndex = (y * width + x) * channels + c;
                
                // Copy data
                openglData[openglIndex] = cimgImage(x,y,0,c);
            }
        }
    }
}

OpenGLTexture::OpenGLTexture(const std::string& path)
    : m_Path(path)
{
    int width, height, channels;

    // CIMG
    cimg_library::CImg<unsigned char> image(path.c_str());
    channels = image.spectrum();
    width = image.width();
    height = image.height();
    std::vector<unsigned char> openglData(width * height * channels);
    auto data = openglData.data();
    convertCImgToOpenGL(image, data, width, height, channels);

  
    // STB IMAGE
    // stbi_set_flip_vertically_on_load(1);
    // stbi_uc* data = nullptr;
    // data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (data)
    {
        std::cout << "Creating texture\n";
        m_IsLoaded = true;

        m_Width = width;
        m_Height = height;

        GLenum internalFormat = 0, dataFormat = 0;
        if (channels == 4)
        {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else if (channels == 3)
        {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        std::cout << "Channels " << channels << "\n";

        m_InternalFormat = dataFormat;//internalFormat;
        m_DataFormat = dataFormat;


        std::cout << "Create\n";
        //glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glGenTextures(1, &m_RendererID);
        std::cout << "Storage\n";
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        //glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
        

        std::cout << "Params\n";
        // glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // unsigned char * first = image._data;
        // unsigned char* second = (image._spectrum>1)?image.data(0,0,0,1) : first;
        // unsigned char* third = (image._spectrum>2)?image.data(0,0,0,2) : first;
        // unsigned char* fourth = (image._spectrum>2)?image.data(0,0,0,3) : first;
        
        // int nPixels = width*height;
        // unsigned char allstrings[nPixels*channels];
        // //strcpy(allstrings,first);
        // memcpy(allstrings,first,nPixels);
        // memcpy(allstrings+nPixels,second,nPixels);
        // memcpy(allstrings+nPixels*2,third,nPixels);
        //memcpy(allstrings+nPixels*3,fourth,nPixels);

        std::cout << "Text image\n";
        //glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
        std::cout << "Creating texture exit\n";
    }
}

OpenGLTexture::~OpenGLTexture()
{
    glDeleteTextures(1, &m_RendererID);
}

void OpenGLTexture::SetData(void* data, uint32_t size)
{
    uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
    assert(size == m_Width * m_Height * bpp); // "Data must be entire texture!"
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

void OpenGLTexture::Bind(uint32_t slot) const
{
    //glBindTextureUnit(slot, m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}