#ifndef BLUEMARBLE_QUADTREEINDEX
#define BLUEMARBLE_QUADTREEINDEX

#include "ISpatialIndex.h"
#include "IPersistable.h"

namespace BlueMarble
{
    // Forwards declaration
    class QuadTreeNode;

    class QuadTreeIndex : public ISpatialIndex, public IPersistable
    {
    public:
        QuadTreeIndex(const Rectangle& rootBounds, int maxDepth=10);
        ~QuadTreeIndex();

        virtual void build(const FeatureCollectionPtr& entries) override final;

        virtual void insert(const FeatureId& entry, const Rectangle& bounds) override final;
        virtual void clear() override final;
        
        virtual FeatureIdCollectionPtr query(const Rectangle& area) const override final;
        virtual FeatureIdCollectionPtr queryAll() const override final;

        virtual std::string persistanceId() const { return "quadtree"; }
        virtual bool load(const PersistanceContext& path) override final;
        virtual void save(const PersistanceContext& path) const override final;

        double minimumCellSize() const;

    private:
        void saveJson(const std::string& path) const;
        bool loadJson(const std::string& path);

        std::unique_ptr<QuadTreeNode> m_root;
        double m_maxDepth;
    };
}

#endif /* BLUEMARBLE_QUADTREEINDEX */
