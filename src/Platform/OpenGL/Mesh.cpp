#include "Platform/OpenGL/Mesh.h"

#include <cmath>

namespace
{
    const double kPi = 3.14159265358979323846;
}

Mesh::Mesh(int gridWidth, int gridHeight, double originX, double originY, double sizeX, double sizeY, const std::vector<float>& heights, TexturePtr texture)
    : m_indexCount(0)
{
    if (gridWidth < 2 || gridHeight < 2 || (int)heights.size() != gridWidth * gridHeight)
    {
        return;
    }

    m_shader = std::make_shared<Shader>();
    m_shader->linkProgram("Shaders/basic.vert", "Shaders/basic.frag");

    std::vector<Vertice> vertices;
    vertices.reserve(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            float u = (float)x / (float)(gridWidth - 1);
            float v = (float)y / (float)(gridHeight - 1);
            glm::vec3 pos((float)(originX + u * sizeX), (float)(originY + v * sizeY), heights[y * gridWidth + x]);
            vertices.push_back(Vertice{ pos, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(u, v) });
        }
    }

    std::vector<GLuint> indices;
    indices.reserve((gridWidth - 1) * (gridHeight - 1) * 6);
    for (int y = 0; y < gridHeight - 1; ++y)
    {
        for (int x = 0; x < gridWidth - 1; ++x)
        {
            GLuint topLeft = y * gridWidth + x;
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = (y + 1) * gridWidth + x;
            GLuint bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    m_indexCount = (GLuint)indices.size();

    auto info = std::make_shared<RectGeometryInfo>();
    info->m_hasFill = true;
    info->m_shader = m_shader;
    info->m_texture = texture;

    m_rect = std::make_shared<Rect>(info, vertices, indices);
}

void Mesh::draw(const glm::mat4& viewProjMatrix)
{
    if (!m_rect || m_indexCount == 0) return;

    GLint texIndex = 0;
    m_shader->useProgram();
    m_shader->setInt("texture0", texIndex);
    glm::mat4 mat = viewProjMatrix;
    m_shader->setMat4("viewMatrix", mat);

    m_rect->drawIndex(m_indexCount);
}

void Mesh::setTexture(TexturePtr texture)
{
    m_rect->setTexture(texture);
}

std::vector<float> Mesh::generateProceduralHeights(int gridWidth, int gridHeight, double amplitude)
{
    std::vector<float> heights(gridWidth * gridHeight);
    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            double fx = (double)x / (double)(gridWidth - 1) * 4.0 * kPi;
            double fy = (double)y / (double)(gridHeight - 1) * 4.0 * kPi;
            heights[y * gridWidth + x] = (float)(amplitude * std::sin(fx) * std::cos(fy));
        }
    }
    return heights;
}

TexturePtr Mesh::generateCheckerboardTexture(int size, int checkerSize)
{
    std::vector<unsigned char> pixels(size * size * 3);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            bool light = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            unsigned char value = light ? 220 : 60;
            int idx = (y * size + x) * 3;
            pixels[idx + 0] = value;
            pixels[idx + 1] = light ? value : (unsigned char)(value * 0.6);
            pixels[idx + 2] = light ? value : (unsigned char)(value * 0.3);
        }
    }

    auto texture = std::make_shared<Texture>();
    texture->init(pixels.data(), size, size, 3, GL_UNSIGNED_BYTE, 0);
    return texture;
}
