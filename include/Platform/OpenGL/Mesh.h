#pragma once
#include "Platform/OpenGL/Rect.h"
#include "Platform/OpenGL/Vertice.h"
#include "Platform/OpenGL/Shader.h"
#include "Platform/OpenGL/Texture.h"
#include "glm.hpp"

#include <vector>
#include <memory>

// A simple elevated grid mesh: gridWidth x gridHeight vertices spanning [0, sizeX] x [0, sizeY]
// in the XY plane, with each vertex's Z taken from a supplied height field. This is a first,
// standalone step towards draping map content over real elevation data -- not wired into
// Map/Layer yet, just enough to get a textured, elevated grid on screen.
class Mesh
{
public:
    // heights must have gridWidth*gridHeight entries, row-major (y*gridWidth + x).
    // The grid spans [originX, originX+sizeX] x [originY, originY+sizeY] in map/world coordinates
    // -- e.g. crs()->bounds().xMin()/yMin()/width()/height() -- not a local (0,0)-based offset.
    Mesh(int gridWidth, int gridHeight, double originX, double originY, double sizeX, double sizeY, const std::vector<float>& heights, TexturePtr texture);

    void draw(const glm::mat4& viewProjMatrix);

    void setTexture(TexturePtr texture);

    // Placeholder height field (rolling hills) until real DEM sampling exists
    static std::vector<float> generateProceduralHeights(int gridWidth, int gridHeight, double amplitude);
    // Placeholder texture so the draped grid is visibly warped, independent of any real imagery
    static TexturePtr generateCheckerboardTexture(int size, int checkerSize);

private:
    ShaderPtr m_shader;
    RectPtr m_rect;
    GLuint m_indexCount;
};
typedef std::shared_ptr<Mesh> MeshPtr;
