#ifndef BLUEMARBLE_IFEATURESERIALIZER
#define BLUEMARBLE_IFEATURESERIALIZER

#include "BlueMarbleMaps/Core/Feature.h"

#include <string>

////////////////////// TODO //////////////////////
namespace BlueMarble
{
    typedef std::string DomNode;

    class IFeatureDomSerializer
    {
        public:
            virtual ~IFeatureDomSerializer() = default;

            
            virtual DomNode* serialize(const FeaturePtr& feature) = 0;
            virtual FeaturePtr deSerialize(DomNode* domNode) = 0;
    };
}

#endif /* BLUEMARBLE_IFEATURESERIALIZER */
