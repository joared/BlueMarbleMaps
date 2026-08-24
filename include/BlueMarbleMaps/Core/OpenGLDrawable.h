#pragma once
#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include "glad/glad.h"
#endif
#include <GLFW/glfw3.h>
#include "Drawable.h"
#include "BlueMarbleMaps/Core/Brush.h"
#include "BlueMarbleMaps/Core/Pen.h"
#include "Platform/OpenGL/Batch.h"
#include "Platform/OpenGL/Primitive.h"
#include "Platform/OpenGL/Rect.h"
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

        TexturePtr getFboTexture() const
        {
            return std::make_shared<Texture>(m_fboTexture);
        }

        void makeCurrent() override final
        {
            if (m_window)
            {
                glfwMakeContextCurrent(m_window);
            }

            if (m_framebuffer != 0)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
                glBindTexture(GL_TEXTURE_2D, m_fboTexture);
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // explicit: there's only one context now, framebuffer binding is what actually switches targets
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            
            glViewport(0, 0, m_width, m_height);
        };
        void blitTo(const DrawablePtr& target) override final
        {
            auto glTarget = std::dynamic_pointer_cast<OpenGLDrawable>(target);
            if (!glTarget) return; // nothing sensible to do if the target isn't GL-backed

            if (!m_compositeQuad)
            {
                // Same shape as drawRaster's primitive-creation block: a textured quad, built once and cached.
                // Unlike drawRaster, the texture sampled here (m_fboTexture) is owned by this OpenGLDrawable
                // itself, not by the Texture wrapper, so it's attached non-owning — the quad's Texture won't
                // delete it when the quad (or its cache entry) is destroyed.
                std::vector<Vertice> vertices = {
                    Vertice{ glm::vec3(-1.0f,-1.0f, 0.0f), glm::vec4(1,1,1,1), glm::vec2(0.0f, 0.0f) },
                    Vertice{ glm::vec3( 1.0f,-1.0f, 0.0f), glm::vec4(1,1,1,1), glm::vec2(1.0f, 0.0f) },
                    Vertice{ glm::vec3( 1.0f, 1.0f, 0.0f), glm::vec4(1,1,1,1), glm::vec2(1.0f, 1.0f) },
                    Vertice{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec4(1,1,1,1), glm::vec2(0.0f, 1.0f) },
                };
                std::vector<GLuint> indices = { 0, 1, 2, 0, 2, 3 };

                auto info = std::make_shared<RectGeometryInfo>();
                info->m_hasFill = true;
                info->m_shader = m_basicShader;
                info->m_texture = std::make_shared<Texture>(m_fboTexture); // non-owning wrapper around our own FBO color attachment

                m_compositeQuad = std::make_shared<Rect>(info, vertices, indices);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, glTarget->m_framebuffer);
            glViewport(0, 0, glTarget->m_width, glTarget->m_height);

            GLint texIndex = 0;
            m_basicShader->useProgram();
            m_basicShader->setInt("texture0", texIndex);
            glm::mat4 identity(1.0f);
            m_basicShader->setMat4("viewMatrix", identity); // quad is already in NDC — no projection needed, covers the target regardless of its size

            m_compositeQuad->drawIndex(6); // binds m_fboTexture (via the non-owning wrapper) and draws — same call drawRaster uses

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        };
        DrawablePtr createCompatibleOffscreenDrawable(int width, int height, int colorDepth=4) override final;
        
        // New stuff
        void setProjectionMatrix(const glm::dmat4& proj);
        void setViewMatrix(const glm::dmat4& viewMatrix);
        void setRenderOrigin(const Point& origin);
        glm::dmat4 getProjectionMatrix() { return m_projectionMatrix; };
        glm::dmat4 getViewMatrix() { return m_viewMatrix; };
        Point getRenderOrigin() const { return m_renderOrigin; };

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
        void drawRaster(const RasterGeometryPtr& raster, const Brush& brush, const Rectangle& clip);
        void drawText(int x, int y, const std::string& text, const Color& color, int fontSize = 20, const Color& backgroundColor = Color::transparent());
        Color readPixel(int x, int y);
        void setPixel(int x, int y, const Color& color);
        void clearBuffer() override final;
        void swapBuffers();
        Raster getRaster() override final;
        void flushCache() override final;
    protected:
        #ifndef __EMSCRIPTEN__
        GLFWwindow* m_window;
        #endif
        int m_width;
        int m_height;
    private:
        static glm::mat4x4 transformToMatrix(const Transform& transform);
	    Vertice createPoint(const Point& point, Color color);
        Color getColorFromList(const std::vector<Color>& colors, int index);
        static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

	    std::map<BMID,PrimitivePtr> m_primitives;
        ShaderPtr m_basicShader;
        ShaderPtr m_polyShader;
        ShaderPtr m_lineShader;
        Transform m_transform;
        glm::dmat4 m_viewMatrix;
        glm::dmat4 m_projectionMatrix;
        Point       m_renderOrigin;
        Color m_color;
        BatchPtr lineBatch = nullptr;
        BatchPtr polyBatch = nullptr;
        GLuint m_framebuffer = 0;
        GLuint m_fboTexture = 0;
        RectPtr m_compositeQuad;
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
