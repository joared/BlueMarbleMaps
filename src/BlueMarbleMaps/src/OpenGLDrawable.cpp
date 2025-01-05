#include "OpenGLDrawable.h"
#include "VAO.h"
#include "VBO.h"
#include "IBO.h"
#include "Shader.h"
#include "Texture.h"
#include "stb_image.h"

using namespace BlueMarble;

BlueMarble::OpenGLDrawable::OpenGLDrawable(int width, int height, int colorDepth)
    :m_idSet()
    ,m_transform()
{

}

int BlueMarble::OpenGLDrawable::width() const
{
    return 155;
}

int BlueMarble::OpenGLDrawable::height() const
{
    return 155;
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

void BlueMarble::OpenGLDrawable::drawLine(const std::vector<Point>& points, const Color& color, double width)
{
}

void BlueMarble::OpenGLDrawable::drawPolygon(const std::vector<Point>& points, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::drawRect(const Point& topLeft, const Point& bottomRight, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::drawRect(const Rectangle& rect, const Color& color)
{
}

void BlueMarble::OpenGLDrawable::drawRaster(int x, int y, const Raster& raster, double alpha)
{
    
}

unsigned char* readImage(std::string path, int* width, int* height, int* nrOfChannels)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned char* imgBytes = stbi_load(path.c_str(), width, height, nrOfChannels, STBI_rgb_alpha);

    if (imgBytes == NULL)
    {
        std::cout << "couldn't load texture: " << stbi_failure_reason() << "\n";
        return nullptr;
    }
    return imgBytes;
}

void BlueMarble::OpenGLDrawable::drawRaster(const RasterGeometryPtr& raster, double alpha)
{
    static VAO vao;
    static VBO vbo;
    static IBO ibo;
    static Shader shader;
    static Texture texture;
    static std::vector<Vertice> vertices = { Vertice{glm::vec3(-1.0f,1.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0,0)},
                                          Vertice{glm::vec3(1.0f,1.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(0,1)},
                                          Vertice{glm::vec3(1.0f,-1.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1,1)},
                                          Vertice{glm::vec3(-1.0f,-1.0f,0.0f), glm::vec4(1.0f,1.0f,1.0f,(float)alpha), glm::vec2(1,0)} };
    static std::vector<GLuint> indices = { 0,1,2,
                                           2,3,0 };
    if (!m_idSet.count(raster->getID()))
    {
        Raster& r = raster->raster();
        

        shader.linkProgram("shaders/basic.vert", "shaders/basic.frag");

        GLint texIndex = 0;
        int width, height, channels;
        const unsigned char* data = readImage("C:/Users/Ottop/Onedrive/Skrivbord/goat.jpg", &width, &height, &channels);

        texture.init(r.data(), r.width(), r.height(), r.channels(), GL_UNSIGNED_BYTE, texIndex);

        shader.useProgram();
        shader.setInt("texture0", texIndex);

        vbo.init(vertices);
        ibo.init(indices);
        vao.init();

        vao.bind();
        vbo.bind();
        vao.link(vbo, 0, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)0);
        vao.link(vbo, 1, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
        vao.link(vbo, 2, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
        vbo.unbind();
        m_idSet.insert(raster->getID());
    }
    else
    {
        shader.useProgram();
        shader.setMat4("viewMatrix", glm::mat4(1.0f));
        vao.bind();
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
}
