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
#include "Algorithms.h"

#define SEVERITY_THRESHOLD GL_DEBUG_SEVERITY_LOW

using namespace BlueMarble;

void GLAPIENTRY BlueMarble::OpenGLDrawable::MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{

    std::string severityStr;
        switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            if (SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_LOW || SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_MEDIUM || SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_HIGH)
            {
                return;
            }
            severityStr = "NOTIFICATION";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            if (SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_MEDIUM || SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_HIGH)
            {
                return;
            }
            severityStr = "LOW";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            if (SEVERITY_THRESHOLD == GL_DEBUG_SEVERITY_HIGH)
            {
                return;
            }
            severityStr = "MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            severityStr = "HIGH";
            break;
        }
    std::cout << "---------------------opengl-callback-start------------" << std::endl;
    std::cout << "message: " << message << std::endl;
    std::cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER";
        break;
    }
    std::cout << std::endl;

    std::cout << "id: " << id << std::endl;
    std::cout << "severity: " << severityStr << std::endl;
    std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}

BlueMarble::OpenGLDrawable::OpenGLDrawable(int width, int height, int colorDepth)
    : m_primitives()
    , m_transform()
    , m_width(width)
    , m_height(height)
    , m_window(nullptr)
{
    //glDisable(GL_CULL_FACE);
    glDebugMessageCallback(MessageCallback, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    glViewport(0, 0, m_width, m_height);
    m_basicShader = std::make_shared<Shader>();
    m_basicShader->linkProgram("Shaders/basic.vert", "Shaders/basic.frag");
    m_lineShader = std::make_shared<Shader>();
    m_lineShader->linkProgram("Shaders/line.vert", "Shaders/line.frag");
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

void BlueMarble::OpenGLDrawable::drawLine(const LineGeometryPtr& geometry, const Pen& pen)
{
    if (m_primitives.find(geometry->getID()) == m_primitives.end())
    {
        std::vector<Vertice> vertices;

        std::vector<Point> bounds = geometry->points();
        std::vector<Color> colors = pen.getColors();

        for (int i = 0; i < bounds.size(); i++)
        {
            Color bmColor = getColorFromList(pen.getColors(), i);

            vertices.push_back(createPoint(bounds[i],bmColor));
            if (i < bounds.size()-2)
            {
                Color bmColor = getColorFromList(pen.getColors(), i+1);
                vertices.push_back(createPoint(bounds[i+1], bmColor));
            }
            else
            {
                vertices.push_back(vertices[0]);
            }
        }
        PrimitiveGeometryInfoPtr info = std::make_shared<PrimitiveGeometryInfo>();
        info->m_shader = m_lineShader;
        info->m_hasFill = false;
        Primitive2DPtr primitive = std::make_shared<Primitive2D>(info, vertices);
        m_primitives[geometry->getID()] = primitive;
    }
    std::cout << "Drawing line id: " << geometry->getID() << "\n";
    Primitive2DPtr primitive = m_primitives[geometry->getID()];

    auto center = m_transform.translation();
    double scale = m_transform.scale();
    auto info = OrthographicCameraInformation();

    info.m_far = 100000000000000000.0;
    info.m_height = m_height / scale;
    info.m_width = m_width / scale;
    CameraOrthographic cam = CameraOrthographic(info);
    cam.pan(center.x(), -center.y(), 1.0);
    auto& viewMatrix = cam.calculateTranslations();

    if (primitive->getShader() != nullptr)
    {
        primitive->getShader()->useProgram();
        primitive->getShader()->setMat4("viewMatrix", viewMatrix);
    }

    primitive->drawLine(geometry->points().size()*2, 10);
    
}

void BlueMarble::OpenGLDrawable::drawPolygon(const PolygonGeometryPtr& geometry, const Brush& brush)
{
    if (m_primitives.find(geometry->getID()) == m_primitives.end())
    {
        std::vector<Vertice> vertices;
        std::vector<GLuint> indices;

        std::vector<Point> bounds = geometry->outerRing();
        std::vector<Color> colors = brush.getColors();

        for (int i = 0; i < bounds.size(); i++)
        {
            glm::vec3 pos(bounds[i].x(), bounds[i].y(), 0);

            Color bmColor = getColorFromList(brush.getColors(), i);
            glm::vec4 glColor((float)bmColor.r() / 255, (float)bmColor.g() / 255, (float)bmColor.b() / 255, bmColor.a());

            vertices.push_back(Vertice{pos, glColor});
        }
        std::vector<Vertice> triangles;
        if (Algorithms::triangulatePolygon(vertices, std::vector<Vertice>(), triangles, indices) == false)
        {
            std::cout << "Couldn't draw object due to not being able to triangulate it" << std::endl;
            return;
        }
        
        PrimitiveGeometryInfoPtr info = std::make_shared<PrimitiveGeometryInfo>();
        info->m_shader = m_basicShader;

        Primitive2DPtr primitive = std::make_shared<Primitive2D>(info,vertices,indices);
        m_primitives[geometry->getID()] = primitive;
    }
    std::cout << "Drawing polygon with id: " << geometry->getID() << "\n";
    Primitive2DPtr primitive = m_primitives[geometry->getID()];

    auto center = m_transform.translation();
    double scale = m_transform.scale();
    auto info = OrthographicCameraInformation();

    info.m_far = 100000000000000000.0;
    info.m_height = m_height / scale;
    info.m_width = m_width / scale;
    CameraOrthographic cam = CameraOrthographic(info);
    cam.pan(center.x(), -center.y(), 1.0);
    auto& viewMatrix = cam.calculateTranslations();

    if (primitive->getShader() != nullptr)
    {
        primitive->getShader()->useProgram();
        primitive->getShader()->setMat4("viewMatrix", viewMatrix);
    }
    
    primitive->drawIndex(geometry->outerRing().size());
    
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

void BlueMarble::OpenGLDrawable::drawRaster(const RasterGeometryPtr& raster, const Brush& brush)
{ 
    if (m_primitives.find(raster->getID()) == m_primitives.end())
    {
        std::vector<Vertice> vertices;
        std::vector<GLuint> indices;

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

            Color bmColor = getColorFromList(brush.getColors(), i);
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
        std::vector<Vertice> triangles;
        if (Algorithms::triangulatePolygon(vertices, std::vector<Vertice>(), triangles, indices) == false)
        {
            std::cout << "Couldn't draw object due to not being able to triangulate it" << std::endl;
            return;
        }

        PrimitiveGeometryInfoPtr info = std::make_shared<PrimitiveGeometryInfo>();
        info->m_hasFill = true;

        info->m_shader = m_basicShader;

        GLint texIndex = 0;
        info->m_texture = std::make_shared<Texture>();
        info->m_texture->init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

        info->m_shader->useProgram();
        info->m_shader->setInt("texture0", texIndex);

        Primitive2DPtr primitive = std::make_shared<Primitive2D>(info, vertices, indices);
        m_primitives[raster->getID()] = primitive;
    }
    std::cout << "Drawing raster with id: " << raster->getID() << "\n";
    Primitive2DPtr primitive = m_primitives[raster->getID()];
    auto center = m_transform.translation();
    double scale = m_transform.scale();
    auto info = OrthographicCameraInformation();
    
    info.m_far = 100000000000000000.0;
    info.m_height = m_height/scale;
    info.m_width = m_width/scale;
    CameraOrthographic cam = CameraOrthographic(info);
    cam.pan(center.x(), -center.y(), 1.0);
    auto& viewMatrix = cam.calculateTranslations();

    //cam.zoom(1.0 / scale);

    if (primitive->getShader() != nullptr)
    {
        primitive->getShader()->useProgram();
        primitive->getShader()->setMat4("viewMatrix", viewMatrix);
    }
    primitive->drawIndex(6);
    
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

Vertice BlueMarble::OpenGLDrawable::createPoint(Point& point, Color color)
{
    glm::vec3 pos(point.x(), point.y(), 0);
    glm::vec4 glColor((float)color.r() / 255, (float)color.g() / 255, (float)color.b() / 255, color.a());
    return Vertice{ pos, glColor };
}

Color BlueMarble::OpenGLDrawable::getColorFromList(const std::vector<Color>& colors, int index)
{
    if (index < colors.size())
    {
        return colors[index];
    }
    else if (colors.size())
    {
        return colors.back();
    }
    else
    {
        return Color(0, 0, 0, 1.0f);
    }
}

void BlueMarble::WindowOpenGLDrawable::setWindow(void* window)
{
    m_window = reinterpret_cast<GLFWwindow*>(window);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
}
