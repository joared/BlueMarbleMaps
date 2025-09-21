// #ifndef BLUEMARBLE_ALGORITHM
// #define BLUEMARBLE_ALGORITHM

// #include "BlueMarbleMaps/Core/Feature.h"

// namespace TEMP_BlueMarble
// {
//     namespace Algorithm
//     {
//         class QuadTree; // Forwards declaration

//         class QuadTreeNode
//         {
//             public:
//                 QuadTreeNode(const Rectangle& bounds)
//                     : m_bounds(bounds)
//                     , m_features()
//                 {

//                 }

//                 const std::vector<QuadTreeNode*>& children() { return m_children; }
//                 void addChild(QuadTreeNode* child) 
//                 { 
//                     // assert(this != child);
//                     m_children.push_back(child); 
//                 }

//                 const Rectangle& bounds() const { return m_bounds; }

//                 void addFeature(FeaturePtr feature)
//                 {
//                     m_features.add(feature);
//                 }

//                 bool insertFeature(FeaturePtr feature, double minSize)
//                 {
//                     if (!feature->isStrictlyInside(m_bounds))
//                         return false;
                    
//                     // It is inside our bounds, 
//                     // but first check children before adding
                    
//                     // TODO: it is unnecessary to extend this node if it is empty.
//                     // However, if we do that, we need to reorganize the tree very time 
//                     // a feature is inserted. maybe when the number of features exceeds 4?
//                     // For static datasets, it might be better/faster to first read all features,
//                     // and then call an "insertFeatures" or "setFeatures" method
//                     if (m_children.empty())
//                         extend(minSize);
                    
//                     for (auto child : m_children)
//                     {
//                         if (child->insertFeature(feature, minSize))
//                         {
//                             return true;
//                         }
//                     }

//                     // No child bounds the entire feature, we add it!
//                     m_features.add(feature);
//                     return true;
//                 }

//                 // Creates children (extends the tree)
//                 void extend(double minSize)
//                 {
//                     assert(m_children.empty());

//                     if (m_bounds.width()*0.5 < minSize || m_bounds.height()*0.5 < minSize )
//                     {
//                         // std::cout << "Reached min size: " << minSize << "\n";
//                         return;
//                     }

//                     auto center = m_bounds.center();
//                     // std::cout  << "Center: " << center.x() << ", " << center.y() << "\n";
//                     // std::cout  << "Rect: " << m_bounds.xMin() << ", " << m_bounds.yMin() << ", " << m_bounds.xMax() << ", " << m_bounds.yMax() << "\n";
                    
//                     auto southWest = new QuadTreeNode(Rectangle(m_bounds.xMin(), m_bounds.yMin(), center.x(), center.y()));
//                     auto southEast = new QuadTreeNode(Rectangle(center.x(), m_bounds.yMin(), m_bounds.xMax(), center.y()));
//                     auto northWest = new QuadTreeNode(Rectangle(m_bounds.xMin(), center.y(), center.x(), m_bounds.yMax()));
//                     auto northEast = new QuadTreeNode(Rectangle(center.x(), center.y(), m_bounds.xMax(), m_bounds.yMax()));
//                     addChild(southWest);
//                     addChild(southEast);
//                     addChild(northWest);
//                     addChild(northEast);

//                     // std::cout  << "Added: " << center.x() << ", " << center.y() << "\n";
//                 }

//                 const FeatureCollection& features() { return m_features; }
//                 FeaturePtr getFeature(const Id& id, bool recursive=false) const
//                 {
//                     for (auto f : m_features)
//                     {
//                         if (id == f->id())
//                             return f;
//                     }

//                     if (recursive)
//                     {
//                         for (auto child : m_children)
//                         {
//                             if (auto f = child->getFeature(id))
//                                 return f;
//                         }
//                     }

//                     return nullptr;
//                 }

//                 // Recursively retrives all features
//                 void getAllFeatures(FeatureCollection& features) const
//                 {
//                     for (auto f : m_features)
//                         features.add(f);
                    
//                     for (auto child : m_children)
//                         child->getAllFeatures(features);
//                 }

//                 void getFeaturesInside(const Rectangle& bounds, FeatureCollection& features) const
//                 {
//                     if (!m_bounds.overlap(bounds))
//                         return;

//                     if (bounds.isInside(m_bounds))
//                     {
//                         // If our bounds are completely inside the bounds
//                         // get all features
//                         getAllFeatures(features);
//                         return;
//                     }

//                     for (auto f : m_features)
//                     {
//                         // NOTE: if bounds are cached, this is faster.
//                         // Otherwise probably slower
//                         if (f->bounds().overlap(bounds)
//                             && f->isInside(bounds))
//                         {
//                             features.add(f);
//                         }
//                     }
                        
//                     for (auto child : m_children)
//                     {
//                         child->getFeaturesInside(bounds, features);
//                     }
//                 }

//             private:
//                 Rectangle                  m_bounds;
//                 FeatureCollection          m_features;
//                 std::vector<QuadTreeNode*> m_children;
//                 QuadTree*                  m_owner;
//         };

//         class QuadTree
//         {
//             public:
//                 static Rectangle northOf(const Rectangle& bounds);
//                 static Rectangle southOf(const Rectangle& bounds);
//                 static Rectangle eastOf(const Rectangle& bounds);
//                 static Rectangle westOf(const Rectangle& bounds);

//                 QuadTree(const Rectangle& rootBounds, double minSize);
//                 QuadTreeNode* root() { return m_root; }
//                 double minSize() { return m_minSize; }
//                 void insertFeature(FeaturePtr feature);
//                 FeaturePtr findFeature(const Id& id) const;
//                 void getAllFeatures(FeatureCollection& features);
//                 void getFeaturesInside(const Rectangle& bounds, FeatureCollection& features);                

//             private:
//                 QuadTreeNode* m_root;
//                 double m_minSize;

//         };

//     };
// };

// #endif /* BLUEMARBLE_ALGORITHM */
