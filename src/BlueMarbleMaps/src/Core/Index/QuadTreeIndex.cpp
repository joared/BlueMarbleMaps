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
            , m_children()
        {
            m_children.reserve(4);
        }
        QuadTreeNode(QuadTreeNode&&) = default;
        QuadTreeNode& operator=(QuadTreeNode&&) = default;
        QuadTreeNode(const QuadTreeNode&) = delete;
        QuadTreeNode& operator=(const QuadTreeNode&) = delete;

        const std::vector<QuadTreeNode>& children() const { return m_children; }
        std::vector<QuadTreeNode>& children() { return m_children; }
        const std::vector<Entry>& entries() const { return m_entries; }
        
        QuadTreeNode& emplaceChild(QuadTreeNode&& child) 
        { 
            // assert(this != child);
            m_children.emplace_back(std::move(child)); 
            return m_children.back();
        }

        const Rectangle& bounds() const { return m_bounds; }
        void setBounds(const Rectangle& bounds) { m_bounds = bounds; }

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
            
            for (auto& child : m_children)
            {
                if (child.insert(id, bounds, minSize))
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
            
            emplaceChild(QuadTreeNode(Rectangle(m_bounds.xMin(), m_bounds.yMin(), center.x(), center.y()))); // south west
            emplaceChild(QuadTreeNode(Rectangle(center.x(), m_bounds.yMin(), m_bounds.xMax(), center.y()))); // south east
            emplaceChild(QuadTreeNode(Rectangle(m_bounds.xMin(), center.y(), center.x(), m_bounds.yMax()))); // north west
            emplaceChild(QuadTreeNode(Rectangle(center.x(), center.y(), m_bounds.xMax(), m_bounds.yMax()))); // north east

            // std::cout  << "Added: " << center.x() << ", " << center.y() << "\n";
        }

        // Recursively retrives all features
        void queryAll(const FeatureIdCollectionPtr& featureIds) const
        {
            for (const auto& e : m_entries)
                featureIds->add(e.first);
            
            for (const auto& child : m_children)
                child.queryAll(featureIds);
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
                
            for (const auto& child : m_children)
            {
                child.query(bounds, featureIds);
            }
        }

    private:
        Rectangle                  m_bounds;
        std::vector<Entry>         m_entries;
        std::vector<QuadTreeNode>  m_children;
        // QuadTree*                  m_owner;
};

QuadTreeIndex::QuadTreeIndex(const Rectangle& rootBounds, double minSize)
    : m_root(nullptr)
    , m_minSize(minSize)
{
}

BlueMarble::QuadTreeIndex::~QuadTreeIndex()
{
}

void QuadTreeIndex::build(const FeatureCollectionPtr &entries)
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

bool QuadTreeIndex::load(const PersistanceContext& ctx)
{
    return loadJson(ctx.fileName);;
}

void QuadTreeIndex::save(const PersistanceContext& ctx) const
{
    saveJson(ctx.fileName);
}

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
    for (const auto& child : node->children())
    {
        children.emplace_back(std::move(serializeNode(&child)));
    }

    return data;
}

QuadTreeNode& deserializeNode(const JsonValue& json, QuadTreeNode& nodeOut)
{
    const JsonValue::Object& data = json.asObject();

    // Bounds of this node
    double xMin = data.at("bounds").asObject().at("xMin").asDouble();
    double yMin = data.at("bounds").asObject().at("yMin").asDouble();
    double xMax = data.at("bounds").asObject().at("xMax").asDouble();
    double yMax = data.at("bounds").asObject().at("yMax").asDouble();
    auto bounds = Rectangle(xMin, yMin, xMax, yMax);

    nodeOut.setBounds(bounds);

    const auto& entries = data.at("entries").asArray();
    for (const auto& e : entries)
    {
        int id = e.asObject().at("id").asInteger();
        double xMin = e.asObject().at("bounds").asObject().at("xMin").asDouble();
        double yMin = e.asObject().at("bounds").asObject().at("yMin").asDouble();
        double xMax = e.asObject().at("bounds").asObject().at("xMax").asDouble();
        double yMax = e.asObject().at("bounds").asObject().at("yMax").asDouble();

        nodeOut.add(id, Rectangle(xMin, yMin, xMax, yMax));
    }

    const auto& children = data.at("children").asArray();
    for (auto& c : children)
    {
        auto& child = nodeOut.emplaceChild(QuadTreeNode(Rectangle(0,0,0,0)));
        deserializeNode(c, child);
    }

    return nodeOut;
}

void QuadTreeIndex::saveJson(const std::string &path) const
{
    auto json = std::move(serializeNode(m_root.get()));
    File::writeString(path, json.toString());
}

bool QuadTreeIndex::loadJson(const std::string &path)
{
    auto f = File(path);
    if (!f.isOpen())
    {
        return false;
    }

    JsonValue json = JsonValue::fromString(File::readAsString(path));
    m_root = std::make_unique<QuadTreeNode>(Rectangle(0,0,0,0)); // dummy rect
    deserializeNode(json, *m_root.get());

    return true;
}
