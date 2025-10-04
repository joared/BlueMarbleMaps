#ifndef BLUEMARBLE_ISPATIALINDEX
#define BLUEMARBLE_ISPATIALINDEX

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Feature.h"

namespace BlueMarble
{
    // Interface for spatial index.
    class ISpatialIndex
    {
    public:
        virtual ~ISpatialIndex() = default;

        virtual void build(const FeatureCollectionPtr& entries, const std::string& path) = 0;
        virtual bool load(const std::string& path) = 0;

        virtual void insert(const FeatureId& id, const Rectangle& bounds) = 0;
        virtual void clear() = 0;

        virtual FeatureIdCollectionPtr query(const Rectangle& area) const = 0;
        virtual FeatureIdCollectionPtr queryAll() const = 0;
    };
}

#endif /* BLUEMARBLE_ISPATIALINDEX */
