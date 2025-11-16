#ifndef BLUEMARBLE_DUMMYINDEX
#define BLUEMARBLE_DUMMYINDEX

#include "ISpatialIndex.h"

namespace BlueMarble
{
    class DummyIndex : public ISpatialIndex
    {
    public:
        DummyIndex()
        : m_idToRect()
        {}
        
        virtual void build(const FeatureCollectionPtr& entries, const std::string& path) override final
        {
            for (const auto& f : *entries)
            {
                insert(f->id().featureId(), f->bounds());
            }
        };

        virtual void insert(const FeatureId& entry, const Rectangle& bounds) override final 
        {
            m_idToRect[entry] = bounds;
        };

        virtual void clear() override final {};
        
        virtual FeatureIdCollectionPtr query(const Rectangle& area) const override final
        {
            auto ids = std::make_shared<FeatureIdCollection>();

            for (const auto& el : m_idToRect)
            {
                if (el.second.overlap(area))
                {
                    ids->add(el.first);
                }
            }

            return ids;
        };

        virtual FeatureIdCollectionPtr queryAll() const override final
        {
            auto ids = std::make_shared<FeatureIdCollection>();

            for (const auto& el : m_idToRect)
            {
                ids->add(el.first);
            }

            return ids;
        };

        virtual bool load(const std::string& path) override final { return false; };
    private:
        std::map<FeatureId, Rectangle> m_idToRect;
    };
}

#endif /* BLUEMARBLE_QUADTREEINDEX */
