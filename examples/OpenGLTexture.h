#ifndef OPENGLTEXTURE
#define OPENGLTEXTURE

#include "Texture.h"
#include "glad.h"

namespace BlueMarble
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const std::string& path);
		virtual ~OpenGLTexture();

		virtual uint32_t GetWidth() const override { return m_Width;  }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual const std::string& GetPath() const override { return m_Path; }
		
		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool IsLoaded() const override { return m_IsLoaded; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		std::string m_Path;
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}

#endif /* OPENGLTEXTURE */
