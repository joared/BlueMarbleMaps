#include "BlueMarbleMaps/Core/Index/QuadTreeIndex.h"
#include "BlueMarbleMaps/System/File.h"
#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"


using namespace BlueMarble;

class QuadTree; // Forwards declaration

typedef std::pair<FeatureId, Rectangle> Entry;
class BlueMarble::QuadTreeNode
{
    public:
        QuadTreeNode(const Rectangle& bounds)
            : m_bounds(bounds)
            , m_entries()
        {
        }

        const std::vector<QuadTreeNode*>& children() const { return m_children; }
        const std::vector<Entry>& entries() const { return m_entries; }
        void addChild(QuadTreeNode* child) 
        { 
            // assert(this != child);
            m_children.push_back(child); 
        }

        const Rectangle& bounds() const { return m_bounds; }

        void add(const FeatureId& id, const Rectangle& bounds)
        {
            m_entries.push_back({id, bounds});
        }

        bool insert(const FeatureId& id, const Rectangle& bounds, double minSize)
        {
            if (!m_bounds.isInside(bounds))
                return false;
            
            // It is inside our bounds, 
            // but first check children before adding
            
            // TODO: it is unnecessary to extend this node if it is empty.
            // However, if we do that, we need to reorganize the tree very time 
            // a feature is inserted. maybe when the number of features exceeds 4?
            // For static datasets, it might be better/faster to first read all features,
            // and then call an "insertFeatures" or "setFeatures" method
            if (m_children.empty())
                extend(minSize);
            
            for (auto child : m_children)
            {
                if (child->insert(id, bounds, minSize))
                {
                    return true;
                }
            }

            // No child bounds the entire feature, we add it!
            m_entries.push_back({id, bounds});
            return true;
        }

        // Creates children (extends the tree)
        void extend(double minSize)
        {
            assert(m_children.empty());

            if (m_bounds.width()*0.5 < minSize || m_bounds.height()*0.5 < minSize )
            {
                // std::cout << "Reached min size: " << minSize << "\n";
                return;
            }

            auto center = m_bounds.center();
            // std::cout  << "Center: " << center.x() << ", " << center.y() << "\n";
            // std::cout  << "Rect: " << m_bounds.xMin() << ", " << m_bounds.yMin() << ", " << m_bounds.xMax() << ", " << m_bounds.yMax() << "\n";
            
            auto southWest = new QuadTreeNode(Rectangle(m_bounds.xMin(), m_bounds.yMin(), center.x(), center.y()));
            auto southEast = new QuadTreeNode(Rectangle(center.x(), m_bounds.yMin(), m_bounds.xMax(), center.y()));
            auto northWest = new QuadTreeNode(Rectangle(m_bounds.xMin(), center.y(), center.x(), m_bounds.yMax()));
            auto northEast = new QuadTreeNode(Rectangle(center.x(), center.y(), m_bounds.xMax(), m_bounds.yMax()));
            addChild(southWest);
            addChild(southEast);
            addChild(northWest);
            addChild(northEast);

            // std::cout  << "Added: " << center.x() << ", " << center.y() << "\n";
        }

        // const FeatureCollection& features() { return m_features; }
        // FeaturePtr getFeature(const Id& id, bool recursive=false) const
        // {
        //     for (const auto& e : m_entries)
        //     {
        //         if (id == e.second)
        //             return f;
        //     }

        //     if (recursive)
        //     {
        //         for (auto child : m_children)
        //         {
        //             if (auto f = child->getFeature(id))
        //                 return f;
        //         }
        //     }

        //     return nullptr;
        // }

        // Recursively retrives all features
        void queryAll(const FeatureIdCollectionPtr& featureIds) const
        {
            for (const auto& e : m_entries)
                featureIds->add(e.first);
            
            for (auto child : m_children)
                child->queryAll(featureIds);
        }

        void query(const Rectangle& bounds, const FeatureIdCollectionPtr& featureIds) const
        {
            if (!m_bounds.overlap(bounds))
                return;

            if (bounds.isInside(m_bounds))
            {
                // If our bounds are completely inside the bounds
                // get all features
                queryAll(featureIds);
                return;
            }

            for (const auto& e : m_entries)
            {
                // NOTE: if bounds are cached, this is faster.
                // Otherwise probably slower
                if (e.second.overlap(bounds))
                {
                    featureIds->add(e.first);
                }
            }
                
            for (auto child : m_children)
            {
                child->query(bounds, featureIds);
            }
        }

    private:
        Rectangle                  m_bounds;
        std::vector<Entry>         m_entries;
        std::vector<QuadTreeNode*> m_children;
        QuadTree*                  m_owner;
};



JsonValue serializeNode(const QuadTreeNode* node)
{
    JsonValue::Object data;
    
    data["bounds"] = 
    {
        {"xMin", node->bounds().xMin()},
        {"yMin", node->bounds().yMin()},
        {"xMax", node->bounds().xMax()},
        {"yMax", node->bounds().yMax()}
    };

    data["entries"] = JsonValue::Array();
    auto& entries = data["entries"].get<JsonValue::Array>();
    for (const auto& e : node->entries())
    {
        const auto& b = e.second;
        entries.push_back({
            {"id", (int)e.first},
            {"bounds", {
                {"xMin", b.xMin()},
                {"yMin", b.yMin()},
                {"xMax", b.xMax()},
                {"yMax", b.yMax()}
            }}
        });
    }
    
    data["children"] = JsonValue::Array();
    auto& children = data["children"].get<JsonValue::Array>();
    for (auto child : node->children())
    {
        children.push_back(serializeNode(child));
    }

    return data;
}


QuadTreeIndex::QuadTreeIndex(const Rectangle& rootBounds, double minSize)
    : m_root(new QuadTreeNode(rootBounds))
    , m_minSize(minSize)
{
}

void QuadTreeIndex::build(const FeatureCollectionPtr& entries, const std::string& path)
{
    for (const auto& f : *entries)
    {
        insert(f->id().featureId(), f->bounds());
    }
}

void QuadTreeIndex::insert(const FeatureId &id, const Rectangle &bounds)
{
    if (!m_root->insert(id, bounds, m_minSize))
    {
        std::cout << "Failed to insert feature id: " << id << "\n";
    }
}

FeatureIdCollectionPtr QuadTreeIndex::query(const Rectangle &area) const
{
    auto ids = std::make_shared<FeatureIdCollection>();
    m_root->query(area, ids);

    return ids;
}

FeatureIdCollectionPtr QuadTreeIndex::queryAll() const
{
    auto ids = std::make_shared<FeatureIdCollection>();
    m_root->queryAll(ids);

    return ids;
}

void QuadTreeIndex::clear()
{
}

bool QuadTreeIndex::load(const std::string& path)
{
    return false;
}

// BlueMarble::FeaturePtr QuadTree::findFeature(const Id &id) const
// {
//     return m_root->getFeature(id, true);
// }


