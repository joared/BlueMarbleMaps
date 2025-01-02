#ifndef BLUEMARBLE_TEXTURE
#define BLUEMARBLE_TEXTURE

#include <stdint.h>
#include <string>

namespace BlueMarble
{
    class Texture
    {
    public:
        virtual ~Texture() = default;

        //virtual const TextureSpecification& GetSpecification() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        virtual const std::string& GetPath() const = 0;

        virtual void SetData(void* data, uint32_t size) = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;

        virtual bool IsLoaded() const = 0;

        virtual bool operator==(const Texture& other) const = 0;
    };
}

#endif /* BLUEMARBLE_TEXTURE */
