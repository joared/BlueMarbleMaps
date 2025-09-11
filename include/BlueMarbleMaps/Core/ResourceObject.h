#ifndef BLUEMARBLE_RESOURCEOBJECT
#define BLUEMARBLE_RESOURCEOBJECT

#include "BlueMarbleMaps/Core/EngineObject.h"

#include <map>
#include <memory>
#include <any>

namespace BlueMarble
{
    #define OBJECT_PROPERTY(type, name) public: const type& name() { return m_##name; }\
                                        public: const void name(const type& value) { m_##name = value; }\
                                        private: type m_##name;\

    class ResourceObject; typedef std::shared_ptr<ResourceObject> ResourceObjectPtr;

    using DescriptorType = std::any;

    struct PropertyDescriptor
    {
        std::string name;
        std::function<DescriptorType (ResourceObject*)> get;
        std::function<void(ResourceObject*, DescriptorType)> set;
    };

    typedef std::vector<PropertyDescriptor> ResourceProperties;
    typedef std::shared_ptr<ResourceProperties> ResourcePropertiesPtr;

    #define DEFINE_RESOURCE_PROPERTY(className, propertyName, propertyType)                             \
    {                                                                                                   \
        #propertyName,                                                                                  \
        [](ResourceObject* obj) -> propertyType                                                         \
        {                                                                                               \
            return static_cast<className*>(obj)->propertyName();                                        \
        },                                                                                              \
        [](ResourceObject* obj, propertyType)                                                           \
        {                                                                                               \
            static_cast<className*>(obj)->propertyName(static_cast<propertyType>(value));               \
        }                                                                                               \
    }

    

    class ResourceObject : public EngineObject
    {
        public:
            virtual ~ResourceObject() = default;
            OBJECT_PROPERTY(std::string, name);
            virtual const ResourcePropertiesPtr& getProperties() const { return nullptr; } // TODO make pure virtual
    };

}

#endif /* BLUEMARBLE_RESOURCEOBJECT */
