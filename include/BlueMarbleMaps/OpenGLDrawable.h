#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include "Drawable.h"
#include <set>

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
        void resize(int width, int height);
        void fill(int val);
        void drawCircle(int x, int y, double radius, const Color& color);
        void drawLine(const std::vector<Point>& points, const Color& color, double width = 1.0);
        void drawPolygon(const std::vector<Point>& points, const Color& color);
        void drawRect(const Point& topLeft, const Point& bottomRight, const Color& color);
        void drawRect(const Rectangle& rect, const Color& color); // Utility method, calls the above
        void drawRaster(int x, int y, const Raster& raster, double alpha);
        void drawRaster(const RasterGeometryPtr& raster, double alpha);
        void drawText(int x, int y, const std::string& text, const Color& color, int fontSize = 20, const Color& backgroundColor = Color::transparent());
        Color readPixel(int x, int y);
        void setPixel(int x, int y, const Color& color);
        void swapBuffers();
        RendererImplementation renderer();
    protected:
        std::set<BMID> m_idSet;
        GLFWwindow* m_window;
        Transform m_transform;

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
