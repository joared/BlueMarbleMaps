#include "BlueMarbleMaps/Core/OpenGLDrawable.h"
#include "BlueMarbleMaps/Core/Geometry.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include "Platform/OpenGL/Shader.h"
#include "Platform/OpenGL/VAO.h"
#include "Platform/OpenGL/VBO.h"
#include "Platform/OpenGL/IBO.h"
#include "Platform/OpenGL/Texture.h"
#include "Platform/OpenGL/CameraPerspective.h"
#include "Platform/OpenGL/CameraOrthographic.h"
#include "Platform/OpenGL/Algorithms.h"
#include "Platform/OpenGL/Line.h"
#include "Platform/OpenGL/Polygon.h"
#include "Platform/OpenGL/Rect.h"
#include "Platform/OpenGL/Batch.h"
#include "stb_image.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"

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
    , m_viewMatrix(glm::mat4x4(1))
    , m_projectionMatrix(glm::mat4x4(1))
    , m_color(Color::white())
{
    //glDisable(GL_CULL_FACE);
    glDebugMessageCallback(MessageCallback, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    m_basicShader = std::make_shared<Shader>();
    m_basicShader->linkProgram("Shaders/basic.vert", "Shaders/basic.frag");
    m_lineShader = std::make_shared<Shader>();
    m_lineShader->linkProgram("Shaders/line.vert", "Shaders/line.frag");

    resize(m_width, m_height);
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
    // TODO: insert return statement here
    return m_color;
}

void BlueMarble::OpenGLDrawable::backgroundColor(const Color& color)
{
    m_color = color;
}

const Transform& BlueMarble::OpenGLDrawable::getTransform()
{
    return m_transform;
}

void BlueMarble::OpenGLDrawable::setTransform(const Transform& transform)
{
    m_transform = transform;

    auto center = m_transform.translation();
    double scaleX = m_transform.scaleX();
    double scaleY = m_transform.scaleY();
    double rotation = m_transform.rotation() * DEG_TO_RAD;

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::rotate(view, -(float)rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::scale(view, glm::vec3(scaleX, scaleY, 1.0f));
    view = glm::translate(view, glm::vec3(
        -center.x(),
        -center.y(),
        -1.0/scaleX
    ));

    m_viewMatrix = view;
    
    // Legacy opengl
    // glMatrixMode(GL_MODELVIEW);  // Set to model-view matrix
    // glLoadMatrixf(glm::value_ptr(m_viewMatrix));

    // glBegin(GL_QUADS);
    // glVertex2f(0, 0);
    // glVertex2f(100, 0);
    // glVertex2f(100, 100);
    // glVertex2f(0, 100);
    // glEnd();
}

void BlueMarble::OpenGLDrawable::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    //std::cout << "I shalle be doing a glViewPort resize yes" << "\n";
    glViewport(0, 0, width, height);

    float w2 = width * 0.5;
    float h2 = height * 0.5;
    glm::mat4 proj = glm::ortho(-w2, w2, -h2, h2, -10.0f, 10.0f);
    //glm::mat4 proj = glm::perspectiveFov(40.0f, (float)width, (float)height, -10.0f, 10.0f);
    
    m_projectionMatrix = proj;

    // Legacy opengl
    // glMatrixMode(GL_PROJECTION);
    // glLoadMatrixf(glm::value_ptr(m_projectionMatrix));
}

void BlueMarble::OpenGLDrawable::drawCircle(double cx, double cy, double radius, const Pen& pen, const Brush& brush)
{
    drawArc(cx, cy, radius, radius, 0, pen, brush);
}

void BlueMarble::OpenGLDrawable::drawArc(double cx, double cy, double rx, double ry, double theta, const Pen& pen, const Brush& brush)
{
    
    // #define ARC_SEGMENTS 32

    // static GLuint vao;
    // static GLuint vbo;
    // static std::vector<Vertex> buffer;

    // float angle = 2 * 3.1415926 / float(ARC_SEGMENTS); 
    // float c = cosf(angle);//precalculate the sine and cosine
    // float s = sinf(angle);
    // float t;

    // float x = 1;//we start at angle = theta 
    // float y = 0; 
    // if (brush.getColor().a() != 0.0)
    // {
    //     //applyBrush(brush);

    //     for(int ii = 0; ii < ARC_SEGMENTS; ii++) 
    //     { 
    //         //apply radius and offset
    //         //glVertex2f(x * rx + cx, y * ry + cy);//output vertex 

    //         // Vertex v {}
    //         // buffer.push_back(vertex);

    //         //apply the rotation matrix
    //         t = x;
    //         x = c * x - s * y;
    //         y = s * t + c * y;
    //     } 
    //     glEnd(); 
        
    //     //applyPen(pen);
    // }

    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // // Upload vertex data (could use glBufferSubData or persistent mapping for dynamic)
    // glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(Vertex), buffer.data(), GL_DYNAMIC_DRAW);

    // // Vertex layout: position + color
    // glEnableVertexAttribArray(0); // position
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // glEnableVertexAttribArray(1); // color
    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    // glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(buffer.size()));
    // //glMultiDrawArrays(GL_TRIANGLE_FAN, firsts.data(), counts.data(), counts.size());

    // glBindVertexArray(0);

    // float angle = 2 * 3.1415926 / float(ARC_SEGMENTS); 
    // float c = cosf(angle);//precalculate the sine and cosine
    // float s = sinf(angle);
    // float t;

    // float x = 1;//we start at angle = theta 
    // float y = 0; 

    // glUseProgram(0);
    
    // if (brush.getColor().a() != 0.0)
    // {
    //     applyBrush(brush);
    
    //     glBegin(GL_TRIANGLE_FAN); 
    //     //glVertex2f(cx, cy); // Center of circle
    //     for(int ii = 0; ii < ARC_SEGMENTS; ii++) 
    //     { 
    //         //apply radius and offset
    //         glVertex2f(x * rx + cx, y * ry + cy);//output vertex 

    //         //apply the rotation matrix
    //         t = x;
    //         x = c * x - s * y;
    //         y = s * t + c * y;
    //     } 
    //     glEnd(); 
        
    //     applyPen(pen);
    // }
    
    
    // // x = 1;
    // // y = 0; 
    // // glBegin(GL_LINE_LOOP);
    // // for(int ii = 0; ii < ARC_SEGMENTS; ii++) 
    // // { 
    // //     //apply radius and offset
    // //     glVertex2f(x * rx + cx, y * ry + cy);//output vertex 

    // //     //apply the rotation matrix
    // //     t = x;
    // //     x = c * x - s * y;
    // //     y = s * t + c * y;
    // // }

    // // Better solution for handling line width
    // glBegin(GL_TRIANGLE_STRIP);
    // float lineWidth = (float)pen.getWidth();
    // for (int i = 0; i <= ARC_SEGMENTS; ++i) {
    //     float angle = 2.0f * M_PI * float(i) / float(ARC_SEGMENTS);
    //     float cosA = cos(angle);
    //     float sinA = sin(angle);
    
    //     float innerX = (rx - lineWidth/2.0f) * cosA + cx;
    //     float innerY = (ry - lineWidth/2.0f) * sinA + cy;
    //     float outerX = (rx + lineWidth/2.0f) * cosA + cx;
    //     float outerY = (ry + lineWidth/2.0f) * sinA + cy;
    
    //     glVertex2f(outerX, outerY); // Outer edge
    //     glVertex2f(innerX, innerY); // Inner edge
    // }

    // glEnd(); 
}

void BlueMarble::OpenGLDrawable::drawLine(const LineGeometryPtr& geometry, const Pen& pen)
{
    if (batch == nullptr)
    {
        batch = std::make_shared<Batch>();
        batch->begin();
    }
    std::vector<Vertice> vertices;

    std::vector<Point> bounds = geometry->points();
    std::vector<Color> colors = pen.getColors();

    for (int i = 0; i < bounds.size(); i++)
    {
        Color bmColor = getColorFromList(pen.getColors(), i);

        vertices.push_back(createPoint(bounds[i], bmColor));
    }

    if (geometry->isClosed())
    {
        vertices.push_back(vertices[0]);
    }
    if (vertices.empty()) return;
    batch->submit(vertices);
    

    /*if (m_primitives.find(geometry->getID()) == m_primitives.end())
    {
        std::vector<Vertice> vertices;

        std::vector<Point> bounds = geometry->points();
        std::vector<Color> colors = pen.getColors();

        for (int i = 0; i < bounds.size(); i++)
        {
            Color bmColor = getColorFromList(pen.getColors(), i);

            vertices.push_back(createPoint(bounds[i],bmColor));
        }

        if (geometry->isClosed())
        {
            vertices.push_back(vertices[0]);
        }
        if (vertices.empty()) return;
        LineGeometryInfoPtr info = std::make_shared<LineGeometryInfo>();
        info->m_shader = m_lineShader;
        LinePtr line = std::make_shared<Line>(info, vertices);
        m_primitives[geometry->getID()] = line;
    }
    //std::cout << "Drawing line id: " << geometry->getID() << "\n";
    LinePtr line = std::static_pointer_cast<Line>(m_primitives[geometry->getID()]);
    if (line == nullptr) return;
    auto mat = m_projectionMatrix*m_viewMatrix;
    if (line->getShader() != nullptr)
    {
        line->getShader()->useProgram();
        line->getShader()->setMat4("viewMatrix", mat);
    }

    int add = 1 ? geometry->isClosed() : 0;
    line->drawLine(geometry->points().size() + add, 10);*/
}

void BlueMarble::OpenGLDrawable::drawPolygon(const PolygonGeometryPtr& geometry, const Pen& pen, const Brush& brush)
{
    if (geometry->rings().size() == 0 || geometry->rings()[0].size() < 3)
        return;

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
        if (vertices.empty()) return;
        std::vector<Vertice> triangles;
        std::vector<Vertice> holes;
        if (Algorithms::triangulatePolygon(vertices, holes, triangles, indices) == false)
        {
            std::cout << "Couldn't draw object due to not being able to triangulate it" << std::endl;
            return;
        }
        PolygonGeometryInfoPtr info = std::make_shared<PolygonGeometryInfo>();
        info->m_shader = m_basicShader;

        PolygonPtr polygon = std::make_shared<Polygon>(info,vertices,indices);
        m_primitives[geometry->getID()] = polygon;
    }
    //std::cout << "Drawing polygon with id: " << geometry->getID() << "\n";
    PolygonPtr primitive = std::static_pointer_cast<Polygon>(m_primitives[geometry->getID()]);
    if (primitive == nullptr) return;
    auto mat = m_projectionMatrix*m_viewMatrix;
    if (primitive->getShader() != nullptr)
    {
        primitive->getShader()->useProgram();
        primitive->getShader()->setMat4("viewMatrix", mat);
    }
    
    primitive->drawIndex(geometry->outerRing().size());
}

void BlueMarble::OpenGLDrawable::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
    auto poly = std::make_shared<PolygonGeometry>();
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

        // Color c1 = getColorFromList(brush.getColors(), 0);
        // // Color c2 = getColorFromList(brush.getColors(), 1);
        // // Color c3 = getColorFromList(brush.getColors(), 2);
        // // Color c4 = getColorFromList(brush.getColors(), 3);

        // glm::vec4 glColor((float)c1.r()/255, (float)c1.g() / 255, (float)c1.b() / 255, c1.a());

        // auto b = raster->bounds();
        // float xMin = b.xMin();
        // float xMax = b.xMax();
        // float yMin = b.yMin();
        // float yMax = b.yMax();
        // vertices = { Vertice{glm::vec3(xMin,yMin,0.0f), glColor, glm::vec2(0.0f,0.0f)},
        //                 Vertice{glm::vec3(xMax,yMin,0.0f), glColor, glm::vec2(1.0f,0.0f)},
        //                 Vertice{glm::vec3(xMax,yMax,0.0f), glColor, glm::vec2(1.0f,1.0f)},
        //                 Vertice{glm::vec3(xMin,yMax,0.0f), glColor, glm::vec2(0.0f,1.0f)} };


        std::vector<Point> bounds = raster->bounds().corners();
        std::vector<Color> colors = brush.getColors();

        double minX = raster->bounds().xMin();
        double minY = raster->bounds().yMin();
        
        for (int i = 0; i < bounds.size(); i++)
        {
            glm::vec3 pos(bounds[i].x(), bounds[i].y(), 0);

            Color bmColor = getColorFromList(brush.getColors(), i);
            glm::vec4 glColor((float)bmColor.r()/255, (float)bmColor.g() / 255, (float)bmColor.b() / 255, bmColor.a());

            float texCoordX = i == 0 || i == 3 ? 0.0 : 1.0;
            float texCoordY = i < 2 ? 0.0 : 1.0;
            glm::vec2 textureCoords(texCoordX, texCoordY);

            vertices.push_back(Vertice{ pos, glColor, textureCoords});
        }
        if (vertices.empty()) return;
        std::vector<Vertice> triangles;
        std::vector<Vertice> holes;
        if (Algorithms::triangulatePolygon(vertices, holes, triangles, indices) == false)
        {
            std::cout << "Couldn't draw object due to not being able to triangulate it" << std::endl;
            return;
        }

        RectGeometryInfoPtr info = std::make_shared<RectGeometryInfo>();
        info->m_hasFill = true;

        info->m_shader = m_basicShader;

        GLint texIndex = 0;
        info->m_texture = std::make_shared<Texture>();
        info->m_texture->init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

        info->m_shader->useProgram();
        info->m_shader->setInt("texture0", texIndex);

        RectPtr rect = std::make_shared<Rect>(info, vertices, indices);
        m_primitives[raster->getID()] = rect;
    }
    //std::cout << "Drawing raster with id: " << raster->getID() << "\n";
    RectPtr rect = std::static_pointer_cast<Rect>(m_primitives[raster->getID()]);
    if (rect == nullptr) return;

    auto mat = m_projectionMatrix*m_viewMatrix;
    if (rect->getShader() != nullptr)
    {
        rect->getShader()->useProgram();
        rect->getShader()->setMat4("viewMatrix", mat);
    }
    rect->drawIndex(6);
}

void OpenGLDrawable::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& backgroundColor)
{
    //auto mat = m_projectionMatrix*m_viewMatrix;
    //m_lineShader.setMat4("viewMatrix", mat);
}

Color OpenGLDrawable::readPixel(int x, int y)
{
    Raster raster(m_width, m_height, 4);
    unsigned char* data = (unsigned char*)(raster.data());
    glReadBuffer(GL_FRONT); // FIXME: front or back?
    glReadPixels(x, width()-y, m_width, m_height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                data);
    
    return Color((int)(data[0]),
                 (int)(data[1]),
                 (int)(data[2]),
                 (double)data[3]/255.0);
}

void BlueMarble::OpenGLDrawable::setPixel(int x, int y, const Color& color)
{
    //glBegin(GL_POINTS);
    //glVertex2f(x, height()-y);
    //glEnd();
}

void BlueMarble::OpenGLDrawable::swapBuffers()
{
    auto mat = m_projectionMatrix * m_viewMatrix;

    m_lineShader->useProgram();
    m_lineShader->setMat4("viewMatrix", mat);
    
    
    batch->end();
    batch->flush();
    glfwSwapBuffers(m_window);
    batch->begin();
}

void BlueMarble::OpenGLDrawable::clearBuffer()
{
    glClearColor(m_color.r()/255.0f, 
                 m_color.g()/255.0f, 
                 m_color.b()/255.0f, 
                 m_color.a());
    glClear(GL_COLOR_BUFFER_BIT);
}

Raster BlueMarble::OpenGLDrawable::getRaster()
{
    Raster raster(m_width, m_height, 4);
    unsigned char* data = (unsigned char*)(raster.data());
    glReadBuffer(GL_FRONT); // FIXME: front or back?
    glReadPixels(0, 0, m_width, m_height,
                GL_RGBA, GL_UNSIGNED_BYTE,
                data);
    
    return raster;
}
RendererImplementation BlueMarble::OpenGLDrawable::renderer()
{
    return RendererImplementation();
}

glm::mat4x4 BlueMarble::OpenGLDrawable::transformToMatrix(const Transform& transform)
{
    // auto center = transform.translation();
    // auto rotation = transform.rotation();
    // double scale = transform.scale();

    throw std::exception(); // Not implemented
    glm::mat4x4 mat;
    // TODO

    return mat;
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

void BlueMarble::WindowOpenGLDrawable::setWindow(void *window)
{
    m_window = reinterpret_cast<GLFWwindow*>(window);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
}
