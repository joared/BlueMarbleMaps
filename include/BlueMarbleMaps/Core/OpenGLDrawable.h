#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include "Drawable.h"
#include "BlueMarbleMaps/Core/Brush.h"
#include "BlueMarbleMaps/Core/Pen.h"
#include "Platform/OpenGL/Batch.h"
#include "Platform/OpenGL/Primitive.h"
#include <map>

namespace BlueMarble
{
    class OpenGLDrawable : public virtual Drawable
    {
    public:
        OpenGLDrawable(int width, int height, int colorDepth = 4);
        OpenGLDrawable(const Drawable& drawable) = delete;
        ~OpenGLDrawable() = default;
        // Properties
        int width() const;
        int height() const;
        const Color& backgroundColor();
        virtual void backgroundColor(const Color& color);

        // Methods
        const Transform& getTransform();
        void setTransform(const Transform& transform);
        void beginBatches();
        void endBatches();
        void resize(int width, int height);
        void drawCircle(double x, double y, double radius, const Pen& pen, const Brush& brush);
        void drawArc(double cx, double cy, double rx, double ry, double theta, const Pen& pen, const Brush& brush);
        void drawLine(const LineGeometryPtr& geometry, const Pen& pen);
        void drawPolygon(const PolygonGeometryPtr& geometry, const Pen& pen, const Brush& brush);
        void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
        void drawRect(const Rectangle& rect, const Color& color); // Utility method, calls the above
        void drawRaster(const RasterGeometryPtr& raster, const Brush& brush);
        void drawText(int x, int y, const std::string& text, const Color& color, int fontSize = 20, const Color& backgroundColor = Color::transparent());
        Color readPixel(int x, int y);
        void setPixel(int x, int y, const Color& color);
        void clearBuffer() override final;
        void swapBuffers();
        Raster getRaster() override final;
        RendererImplementation renderer();
    protected:
        GLFWwindow* m_window;
        int m_width;
        int m_height;
    private:
        static glm::mat4x4 transformToMatrix(const Transform& transform);
	Vertice createPoint(Point& point, Color color);
        Color getColorFromList(const std::vector<Color>& colors, int index);
        static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	    std::map<BMID,PrimitivePtr> m_primitives;
        ShaderPtr m_basicShader;
        ShaderPtr m_lineShader;
        Transform m_transform;
        glm::mat4x4 m_viewMatrix;
        glm::mat4x4 m_projectionMatrix;
        Color m_color;
        BatchPtr lineBatch;
        BatchPtr polyBatch;
    };
    typedef std::shared_ptr<OpenGLDrawable> OpenGLDrawablePtr;

    class BitmapOpenGLDrawable
        : public virtual OpenGLDrawable
        , public virtual BitmapDrawable
    {
    public:
        using OpenGLDrawable::OpenGLDrawable;
    };
    typedef std::shared_ptr<BitmapOpenGLDrawable> OpenGLBitmapDrawablePtr;

    class WindowOpenGLDrawable
        : public virtual OpenGLDrawable
        , public virtual WindowDrawable
    {
    public:
        using OpenGLDrawable::OpenGLDrawable;
        void setWindow(void* window);
    };
    typedef std::shared_ptr<WindowOpenGLDrawable> OpenGLWindowDrawablePtr;
}
