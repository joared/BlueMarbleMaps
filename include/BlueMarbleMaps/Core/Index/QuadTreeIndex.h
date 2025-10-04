#ifndef BLUEMARBLE_QUADTREEINDEX
#define BLUEMARBLE_QUADTREEINDEX

#include "ISpatialIndex.h"

namespace BlueMarble
{
    // Forwards declaration
    class QuadTreeNode;

    class QuadTreeIndex : public ISpatialIndex
    {
    public:
        QuadTreeIndex(const Rectangle& rootBounds, double minSize);
        
        virtual void build(const FeatureCollectionPtr& entries, const std::string& path) override final;

        virtual void insert(const FeatureId& entry, const Rectangle& bounds) override final;
        virtual void clear() override final;
        
        virtual FeatureIdCollectionPtr query(const Rectangle& area) const override final;
        virtual FeatureIdCollectionPtr queryAll() const override final;

        virtual bool load(const std::string& path) override final;
    private:
        QuadTreeNode* m_root;
        double m_minSize;
    };
}

#endif /* BLUEMARBLE_QUADTREEINDEX */
