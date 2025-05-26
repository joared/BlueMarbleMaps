#include "Core/OpenGLDrawable.h"

#include "BlueMarbleMaps/Core/Geometry.h"

#include "VAO.h"
#include "VBO.h"
#include "IBO.h"
#include "Shader.h"
#include "Texture.h"
#include "stb_image.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "CameraPerspective.h"
#include "CameraOrthographic.h"

using namespace BlueMarble;

BlueMarble::OpenGLDrawable::OpenGLDrawable(int width, int height, int colorDepth)
    : m_idSet()
    , m_transform()
    , m_width(width)
    , m_height(height)
{
    //glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    glViewport(0, 0, m_width, m_height);
}

int BlueMarble::OpenGLDrawable::width() const
{
    return m_width;
}

int BlueMarble::OpenGLDrawable::height() const
{
    return m_height;
}

const Color& BlueMarble::OpenGLDrawable::backgroundColor()
{
    return Color();
    // TODO: insert return statement here
}

void BlueMarble::OpenGLDrawable::backgroundColor(const Color& color)
{
}

const Transform& BlueMarble::OpenGLDrawable::getTransform()
{
    return Transform();
    // TODO: insert return statement here
}

void BlueMarble::OpenGLDrawable::setTransform(const Transform& transform)
{
    m_transform = transform;
}

void BlueMarble::OpenGLDrawable::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    std::cout << "I shalle be doing a glViewPort resize yes" << "\n";
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(m_window);
}

void BlueMarble::OpenGLDrawable::fill(int val)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.3f, 0.2f, 1.0f);
}

void BlueMarble::OpenGLDrawable::drawCircle(int x, int y, double radius, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::drawLine(const LineGeometryPtr& geometry, const Color& color, double width)
{
}

void BlueMarble::OpenGLDrawable::drawPolygon(const PolygonGeometryPtr& geometry, const Color& color)
{
    if (m_idSet.find(geometry->getID()) == m_idSet.end())
    {

    }
    else
    {

    }
}

void BlueMarble::OpenGLDrawable::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::drawRect(const Rectangle& rect, const Color& color)
{
}

unsigned char* readImage(std::string path, int* width, int* height, int* nrOfChannels)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned char* imgBytes = stbi_load(path.c_str(), width, height, nrOfChannels, 0);

    if (imgBytes == NULL)
    {
        std::cout << "couldn't load texture: " << stbi_failure_reason() << "\n";
        return nullptr;
    }
    return imgBytes;
}

void BlueMarble::OpenGLDrawable::drawRaster(const RasterGeometryPtr& raster, const Brush& brush, const Pen& pen)
{
    static VAO vao;
    static VBO vbo;
    static IBO ibo;
    static Shader shader;
    static Texture texture;
    
    static std::vector<Vertice> vertices;
    static std::vector<GLuint> indices; 

    

    if (!m_idSet.count(raster->getID()))
    {
        Raster& r = raster->raster();
        int w = (float)r.width();
        int h = (float)r.height();

        std::vector<Point> bounds = raster->bounds().corners();
        std::vector<Color> colors = brush.getColors();

        double minX = raster->bounds().xMin();
        double minY = raster->bounds().yMin();
        
        for (int i = 0; i < bounds.size(); i++)
        {
            glm::vec3 pos(bounds[i].x(), bounds[i].y(), 0);

            Color bmColor;
            if (i < colors.size())
            {
                bmColor = colors[i];
            }
            else if(colors.size())
            {
                bmColor = colors.back();
            }
            else
            {
                bmColor = Color(0,0,0,1.0f);
            }
            glm::vec4 glColor((float)bmColor.r()/255, (float)bmColor.g() / 255, (float)bmColor.b() / 255, bmColor.a());

            float texCoordX = (pos.x - minX) / (float)w;
            float texCoordY = (pos.y - minY) / (float)h;
            glm::vec2 textureCoords(texCoordX, texCoordY);

            vertices.push_back(Vertice{ pos, glColor, textureCoords});
        }
              
        /* vertices = {Vertice{glm::vec3(0.0f,0.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)1.0), glm::vec2(0.0f,1.0f)},
                     Vertice{glm::vec3(w,0.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)1.0), glm::vec2(1.0f,1.0f)},
                     Vertice{glm::vec3(w,-h,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)1.0), glm::vec2(1.0f,0.0f)},
                     Vertice{glm::vec3(0.0f,-h,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)1.0), glm::vec2(0.0f,0.0f)} };
        */
        indices = { 0,1,2,
                    2,3,0 };

        shader.linkProgram("Shaders/basic.vert", "Shaders/basic.frag");

        GLint texIndex = 0;
        texture.init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

        shader.useProgram();
        shader.setInt("texture0", texIndex);

        vbo.init(vertices);
        ibo.init(indices);
        vao.init();

        vao.bind();
        vbo.bind();
        vao.link(vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
        vao.link(vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
        vao.link(vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
        vbo.unbind();
        m_idSet.insert(raster->getID());
    }
    else
    {
        auto center = m_transform.translation();
        double scale = m_transform.scale();
        // auto info = PerspectiveCamerInformation();
        // info.m_far = 100000000000000000.0;
        // CameraPerspective cam = CameraPerspective(info);
        // cam.pan(center.x(), -center.y(), 1000.0/scale);
        auto info = OrthographicCameraInformation();
        
        info.m_far = 100000000000000000.0;
        info.m_height = m_height/scale;
        info.m_width = m_width/scale;
        CameraOrthographic cam = CameraOrthographic(info);
        cam.pan(center.x(), -center.y(), 1.0);
        auto& viewMatrix = cam.calculateTranslations();

        //cam.zoom(1.0 / scale);
        shader.useProgram();
        shader.setMat4("viewMatrix", viewMatrix);
        vao.bind();
        vbo.bind();
        ibo.bind();
        texture.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vao.unbind();
        vbo.unbind();
        ibo.unbind();
        texture.unbind();
    }
}

void BlueMarble::OpenGLDrawable::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& backgroundColor)
{
}

Color BlueMarble::OpenGLDrawable::readPixel(int x, int y)
{
    return Color();
}

void BlueMarble::OpenGLDrawable::setPixel(int x, int y, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

RendererImplementation BlueMarble::OpenGLDrawable::renderer()
{
    return RendererImplementation();
}

void BlueMarble::WindowOpenGLDrawable::setWindow(void* window)
{
    m_window = reinterpret_cast<GLFWwindow*>(window);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
}
