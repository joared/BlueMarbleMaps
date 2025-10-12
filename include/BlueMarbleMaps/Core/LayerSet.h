#ifndef BLUEMARBLE_LAYERSET
#define BLUEMARBLE_LAYERSET

#include "BlueMarbleMaps/Core/Layer.h"

namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    typedef std::shared_ptr<Map> MapPtr;
    
    class LayerSet
        : public Layer
    {
    public:
        LayerSet(bool createdefaultVisualizers = true);
        virtual ~LayerSet() = default;
    }

#endif /* BLUEMARBLE_LAYERSET */