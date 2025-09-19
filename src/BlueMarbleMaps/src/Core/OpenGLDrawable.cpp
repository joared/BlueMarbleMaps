#include "BlueMarbleMaps/Core/OpenGLDrawable.h"
#include "BlueMarbleMaps/Core/Geometry.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include "Platform/OpenGL/VAO.h"
#include "Platform/OpenGL/VBO.h"
#include "Platform/OpenGL/IBO.h"
#include "Platform/OpenGL/Texture.h"
#include "stb_image.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"
#include "Platform/OpenGL/CameraPerspective.h"
#include "Platform/OpenGL/CameraOrthographic.h"

using namespace BlueMarble;



void uploadAndDraw(GLuint vao, 
                   GLuint vbo, 
                   const std::vector<Vertex>& buffer, 
                   const std::vector<GLint>& firsts, 
                   std::vector<GLsizei>& counts, 
                   GLenum mode) 
{
    if (buffer.empty()) return;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Upload vertex data (could use glBufferSubData or persistent mapping for dynamic)
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(Vertex), buffer.data(), GL_DYNAMIC_DRAW);

    // Vertex layout: position + color
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    //glDrawArrays(mode, 0, static_cast<GLsizei>(buffer.size()));
    glMultiDrawArrays(mode, firsts.data(), counts.data(), counts.size());

    glBindVertexArray(0);
}

BlueMarble::OpenGLDrawable::OpenGLDrawable(int width, int height, int colorDepth)
    : m_idSet()
    , m_transform()
    , m_width(width)
    , m_height(height)
    , m_viewMatrix(glm::mat4x4(1))
    , m_projectionMatrix(glm::mat4x4(1))
    , m_color(Color::white())
    , m_lineVAO()
    , m_lineVBO()
    , m_lineShader()
    , m_lineBuffer()
    , m_lineFirsts()
    , m_lineCounts()
    , m_lineFirstCounter(0)
{
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    
    // initialize LineGeometry stuff
    m_lineShader.linkProgram("Shaders/polygon.vert", "Shaders/polygon.frag");
    glGenVertexArrays(1, &m_lineVAO);  glGenBuffers(1, &m_lineVBO);

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
    std::cout << "I shalle be doing a glViewPort resize yes" << "\n";
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
    
    #define ARC_SEGMENTS 32

    static GLuint vao;
    static GLuint vbo;
    static std::vector<Vertex> buffer;

    float angle = 2 * 3.1415926 / float(ARC_SEGMENTS); 
    float c = cosf(angle);//precalculate the sine and cosine
    float s = sinf(angle);
    float t;

    float x = 1;//we start at angle = theta 
    float y = 0; 
    if (brush.getColor().a() != 0.0)
    {
        //applyBrush(brush);

        for(int ii = 0; ii < ARC_SEGMENTS; ii++) 
        { 
            //apply radius and offset
            //glVertex2f(x * rx + cx, y * ry + cy);//output vertex 

            // Vertex v {}
            // buffer.push_back(vertex);

            //apply the rotation matrix
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
        } 
        glEnd(); 
        
        //applyPen(pen);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Upload vertex data (could use glBufferSubData or persistent mapping for dynamic)
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(Vertex), buffer.data(), GL_DYNAMIC_DRAW);

    // Vertex layout: position + color
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(buffer.size()));
    //glMultiDrawArrays(GL_TRIANGLE_FAN, firsts.data(), counts.data(), counts.size());

    glBindVertexArray(0);

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
    if (geometry->points().size() < 2)
        return;

    #define LINE_BUFFER_MAX_SIZE 200000
    if (m_lineBuffer.size() > LINE_BUFFER_MAX_SIZE)
    {
        BMM_DEBUG() << "Line buffer overflow! Ignoring drawLine calls\n";
        return;
    }

    const auto& c = pen.getColor();
    auto glmColor = glm::vec4((float)c.r()/255.0f,
                              (float)c.g()/255.0f,
                              (float)c.b()/255.0f,
                              (float)c.a());
    std::vector<Vertex> coloredPoly;
    for (const auto& p : geometry->points())
    {
        auto v = Vertex{glm::vec3(p.x(), p.y(), 0.0), glmColor};
        coloredPoly.push_back(v);
    }
    if (geometry->isClosed())
    {
        // Closed line
        coloredPoly.push_back(coloredPoly[0]);
    }


    m_lineFirsts.push_back(m_lineFirstCounter);
    m_lineCounts.push_back(coloredPoly.size());
    m_lineFirstCounter += coloredPoly.size();
    
    m_lineBuffer.insert(m_lineBuffer.end(), coloredPoly.begin(), coloredPoly.end());
    
    // Legacy
    // glUseProgram(0);
    
    // bool isFromColorDefined = pen.getFromColor().isDefined(); 
    // bool isFromWidthDefined = pen.getFromWidth() != -1; // Does not work in legacy OpenGL
    // if (isFromColorDefined || isFromWidthDefined)
    // {
    //     Pen p = pen;
    //     if (isFromColorDefined) p.setColor(p.getFromColor());
    //     if (isFromWidthDefined) p.setWidth(p.getFromWidth()); // Does not work in legacy OpenGL
    //     applyPen(p);
    // }
    // else
    // {
    //     applyPen(pen);
    // }

    // int size = geometry->points().size();
    // double totalLength = geometry->length();
    // double currLength = 0.0;
    // if (geometry->isClosed())
    // {
    //     glBegin(GL_LINE_LOOP);

    //     for (int i=0; i<size; i++)
    //     {
    //         const auto& p1 = geometry->points()[i];

    //         if (i > 0 && (isFromColorDefined || isFromWidthDefined))
    //         {
    //             const auto& pPrev = geometry->points()[i-1];
    //             currLength += (p1-pPrev).length();
    //             double progress = currLength/totalLength;

    //             Pen p = pen;
    //             if (isFromColorDefined) p.setColor(p.getFromColor() + (p.getColor()-p.getFromColor())*progress);
    //             if (isFromWidthDefined) p.setWidth(p.getFromWidth() + (p.getWidth()-p.getFromWidth())*progress); // Does not work in legacy OpenGL
    //             applyPen(p);
    //         }

    //         glVertex2f((GLfloat)p1.x(), (GLfloat)p1.y());
    //     }
    // }
    // else
    // {
    //     glBegin(GL_LINES);

    //     for (int i=0; i<size-1; i++) 
    //     {
    //         const auto& p1 = geometry->points()[i];
    //         const auto& p2 = geometry->points()[i+1];

    //         glVertex2f((GLfloat)p1.x(), (GLfloat)p1.y());
    //         glVertex2f((GLfloat)p2.x(), (GLfloat)p2.y());

    //         if (isFromColorDefined || isFromWidthDefined)
    //         {
    //             currLength += (p2-p1).length();
    //             double progress = currLength/totalLength;

    //             Pen p = pen;
    //             if (isFromColorDefined) p.setColor(p.getFromColor() + (p.getColor()-p.getFromColor())*progress);
    //             if (isFromWidthDefined) p.setWidth(p.getFromWidth() + (p.getWidth()-p.getFromWidth())*progress); // Does not work in legacy OpenGL
                
    //             applyPen(p);
    //         }
    //     }
    // }

    // glEnd();
}

void BlueMarble::OpenGLDrawable::drawPolygon(const PolygonGeometryPtr& geometry, const Pen& pen, const Brush& brush)
{
    if (geometry->rings().size() == 0 || geometry->rings()[0].size() < 3)
        return;

    // Legacy
    // glUseProgram(0);
    // applyBrush(brush);
              
    // glBegin(GL_POLYGON);  // Or GL_LINE_LOOP if you don't want it filled

    // // Outer ring
    // for (const auto& point : geometry->outerRing()) 
    // {
    //     glVertex2f((GLfloat)point.x(), (GLfloat)point.y());
    // }
    // // TODO: inner rings

    // glEnd();

    

    // Modern OpenGL
    // static Shader shader;
    // static bool initialized = false;
    // static std::vector<Vertex> polygonBuffer;
    // // static VAO vao;
    // // static VBO vbo;
    // static GLuint vao;
    // static GLuint vbo;
    // if (!initialized)
    // {
    //     shader.linkProgram("Shaders/polygon.vert", "Shaders/polygon.frag");
    //     glGenVertexArrays(1, &vao);  glGenBuffers(1, &vbo);
    // }

    // const auto& c = brush.getColor();
    // auto glmColor = glm::vec4((float)c.r()/255.0f,
    //                           (float)c.g()/255.0f,
    //                           (float)c.b()/255.0f,
    //                           (float)c.a());
    // std::vector<Vertex> coloredPoly;
    // for (const auto& p : geometry->outerRing())
    // {
    //     auto v = Vertex{glm::vec3(p.x(), p.y(), 0.0), glmColor};
    //     coloredPoly.push_back(v);
    // }
    
    // polygonBuffer.insert(polygonBuffer.end(), coloredPoly.begin(), coloredPoly.end());

    // auto mat = m_projectionMatrix*m_viewMatrix;
    // shader.useProgram();
    // shader.setMat4("viewMatrix", mat);

    // uploadAndDraw(vao, vbo, polygonBuffer, GL_TRIANGLES);
    // polygonBuffer.clear();

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

void BlueMarble::OpenGLDrawable::drawRaster(const RasterGeometryPtr& raster, double alpha)
{
    static bool initialized = false;
    static VAO vao;
    static VBO vbo;
    static IBO ibo;
    static Shader shader;
    
    static std::vector<Vertice> vertices;
    static std::vector<GLuint> indices;

    if (!initialized)
    {
        shader.linkProgram("Shaders/basic.vert", "Shaders/basic.frag");
        vao.init();
        vbo.init(vertices);
        vao.bind();
        vbo.bind();
        vao.link(vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
        vao.link(vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
        vao.link(vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
        indices = { 0,1,2,
                    2,3,0 };
        ibo.init(indices);

        initialized = true;
    }

    const Raster& r = raster->raster();
    // float w = (float)r.width() * raster->cellWidth();
    // float h = (float)r.height() * abs(raster->cellHeight());
            
    auto b = raster->bounds();
    float xMin = b.xMin();
    float xMax = b.xMax();
    float yMin = b.yMin();
    float yMax = b.yMax();
    vertices = { Vertice{glm::vec3(xMin,yMin,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,0.0f)},
                    Vertice{glm::vec3(xMax,yMin,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,0.0f)},
                    Vertice{glm::vec3(xMax,yMax,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,1.0f)},
                    Vertice{glm::vec3(xMin,yMax,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,1.0f)} };

    

    GLint texIndex = 0;
    Texture texture;
    texture.init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

    vao.bind();
    vbo.bind();
    ibo.bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertice) * vertices.size(), &vertices[0], GL_STREAM_DRAW);

    auto mat = m_projectionMatrix*m_viewMatrix;
    shader.useProgram();
    shader.setMat4("viewMatrix", mat);
    shader.setInt("texture0", texIndex);

    texture.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    vao.unbind();
    vbo.unbind();
    ibo.unbind();
    texture.unbind();

    // Old
    // if (!m_idSet.count(raster->getID()))
    // {
    //     const Raster& r = raster->raster();
    //     // float w = (float)r.width() * raster->cellWidth();
    //     // float h = (float)r.height() * abs(raster->cellHeight());
              
    //     auto b = raster->bounds();
    //     float xMin = b.xMin();
    //     float xMax = b.xMax();
    //     float yMin = b.yMin();
    //     float yMax = b.yMax();
    //     vertices = { Vertice{glm::vec3(xMin,yMin,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,0.0f)},
    //                  Vertice{glm::vec3(xMax,yMin,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,0.0f)},
    //                  Vertice{glm::vec3(xMax,yMax,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,1.0f)},
    //                  Vertice{glm::vec3(xMin,yMax,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,1.0f)} };
    //     // vertices = { Vertice{glm::vec3(0.0f,0.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,0.0f)},
    //     //              Vertice{glm::vec3(w,0.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,0.0f)},
    //     //              Vertice{glm::vec3(w,h,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1.0f,1.0f)},
    //     //              Vertice{glm::vec3(0.0f,h,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0.0f,1.0f)} };
    //     indices = { 0,1,2,
    //                 2,3,0 };

    //     shader.linkProgram("Shaders/basic.vert", "Shaders/basic.frag");

    //     GLint texIndex = 0;
    //     texture.init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

    //     shader.useProgram();
    //     shader.setInt("texture0", texIndex);

    //     vbo.init(vertices);
    //     ibo.init(indices);
    //     vao.init();

    //     vao.bind();
    //     vbo.bind();
    //     vao.link(vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
    //     vao.link(vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
    //     vao.link(vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
    //     vbo.unbind();
    //     m_idSet.insert(raster->getID());
    // }

    // auto mat = m_projectionMatrix*m_viewMatrix;

    // shader.useProgram();
    // shader.setMat4("viewMatrix", mat);
    // vao.bind();
    // vbo.bind();
    // ibo.bind();
    // texture.bind();
    // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    // vao.unbind();
    // vbo.unbind();
    // ibo.unbind();
    // texture.unbind();
}

void OpenGLDrawable::drawText(int x, int y, const std::string& text, const Color& color, int fontSize, const Color& backgroundColor)
{
    // Faking "flush"

    // Reset any other "applyPen" calls
    Pen pen;
    pen.setColor(Color::white());
    pen.setWidth(1.0);
    pen.setAntiAlias(true);
    applyPen(pen);

    // Use line shader
    m_lineShader.useProgram();
    // Set view matrix
    auto mat = m_projectionMatrix*m_viewMatrix;
    m_lineShader.setMat4("viewMatrix", mat);

    // make the actual draw call
    uploadAndDraw(m_lineVAO, m_lineVBO, m_lineBuffer, m_lineFirsts, m_lineCounts, GL_LINE_STRIP); // GL_LINES, GL_LINE_LOOP
    
    // Clear buffers
    m_lineBuffer.clear();
    m_lineFirsts.clear();
    m_lineCounts.clear();
    m_lineFirstCounter = 0;
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
    glBegin(GL_POINTS);
    glVertex2f(x, height()-y);
    glEnd();
}

void BlueMarble::OpenGLDrawable::swapBuffers()
{
    drawText(0,0,"",Color(), 2);
    glfwSwapBuffers(m_window);
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

void BlueMarble::OpenGLDrawable::applyPen(const Pen& pen)
{
    // Set color
    const auto& color = pen.getColor();
    glColor4f((float)color.r()/255.0f, 
              (float)color.g()/255.0f, 
              (float)color.b()/255.0f, 
              (float)color.a());
    
    // Set line width
    glLineWidth((float)pen.getWidth());

    // Set antialias
    if (pen.getAntiAlias())
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

void BlueMarble::OpenGLDrawable::applyBrush(const Brush& brush)
{
    auto color = brush.getColor();
    glColor4f((float)color.r()/255.0f, 
              (float)color.g()/255.0f, 
              (float)color.b()/255.0f, 
              (float)color.a());

    if (brush.getAntiAlias())
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

void BlueMarble::WindowOpenGLDrawable::setWindow(void *window)
{
    m_window = reinterpret_cast<GLFWwindow*>(window);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);
}
