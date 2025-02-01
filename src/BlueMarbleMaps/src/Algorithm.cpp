#include "BlueMarbleMaps/Utility/Algorithm.h"

using namespace BlueMarble::Algorithm;

BlueMarble::Rectangle QuadTree::northOf(const Rectangle &bounds)
{
    return Rectangle();
}

BlueMarble::Rectangle QuadTree::southOf(const Rectangle &bounds)
{
    return Rectangle();
}

BlueMarble::Rectangle QuadTree::eastOf(const Rectangle &bounds)
{
    return Rectangle();
}

BlueMarble::Rectangle QuadTree::westOf(const Rectangle &bounds)
{
    return Rectangle();
}

QuadTree::QuadTree(const Rectangle& rootBounds, double minSize)
    : m_root(new QuadTreeNode(rootBounds))
    , m_minSize(minSize)
{
}

void QuadTree::insertFeature(FeaturePtr feature)
{
    if (!m_root->insertFeature(feature, m_minSize))
    {
        std::cout << "Failed to insert feature with geometry type: " << (int)feature->geometryType() << "\n";
    }
}

BlueMarble::FeaturePtr QuadTree::findFeature(const Id &id) const
{
    return m_root->getFeature(id, true);
}

void QuadTree::getAllFeatures(FeatureCollection& features)
{
    assert(features.empty());
    m_root->getAllFeatures(features);
}

void QuadTree::getFeaturesInside(const Rectangle& bounds, FeatureCollection& features)
{
    m_root->getFeaturesInside(bounds, features);
}
