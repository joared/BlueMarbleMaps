#ifndef TILELAYER
#define TILELAYER

#include "BlueMarbleMaps/Core/Layer/LayerSet.h"

namespace BlueMarble
{

    class TileLayer : public LayerSet
    {
    public:
        TileLayer();
        virtual void prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
        virtual void update(const MapPtr &crs) override final;
    };

    using TileLayerPtr = std::shared_ptr<TileLayer>;

}

#endif /* TILELAYER */
