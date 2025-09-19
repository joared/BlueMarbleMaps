#include "BlueMarbleMaps/Core/DataSets/FileDataSet.h"

using namespace BlueMarble;

AbstractFileDataSet::AbstractFileDataSet(const std::string& filePath)
    : DataSet() 
    , m_filePath(filePath)
    , m_featureTree(Rectangle(-180.0, -90.0, 180.0, 90.0), 0.05)
    , m_progress(0)
{
}

FeatureEnumeratorPtr AbstractFileDataSet::getFeatures(const FeatureQuery &featureQuery)
{
    auto features = std::make_shared<FeatureEnumerator>();

    if (!isInitialized()) // TODO: move to base class
    {
        return features;
    }

    // Testing QuadTree
    FeatureCollection filteredFeatures;

    m_featureTree.getFeaturesInside(featureQuery.area(), filteredFeatures);
    features->features() = std::move(filteredFeatures.getVector());

    return features;
}

void AbstractFileDataSet::init()
{
    read(m_filePath);
    for (auto f : m_features)
    {
        m_featureTree.insertFeature(f);
    }
    std::cout << "AbstractFileDataSet::init() Data loaded!\n";
}

double AbstractFileDataSet::progress()
{
    return (isInitialized()) ? 1.0 : (double)m_progress;
}

// void AbstractFileDataSet::onGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
// {
//     std::cout << "AbstractFileDataSet::onGetFeaturesRequest\n";
//     if (attributes.size() == 0)
//     {
//         for (auto f : m_features)
//             features.push_back(f);
//         return;
//     }

//     for (auto f : m_features)
//     {
//         for (auto attr : attributes)
//         {
//             if (f->attributes().contains(attr.first))
//             {
//                 try
//                 {
//                     if (f->attributes().get<std::string>(attr.first)
//                         == attributes.get<std::string>(attr.first))
//                     {
//                         // Its a match! Add to feature list
//                         features.push_back(f);
//                     }
//                 }
//                 catch(const std::exception& e)
//                 {
//                     std::cerr << e.what() << '\n';
//                 }
                
                
//             }
//         }
//     }
// }

FeaturePtr AbstractFileDataSet::getFeature(const Id &id)
{
    std::cout << "AbstractFileDataSet::onGetFeatureRequest\n";
    for (auto f : m_features)
    {
        if (f->id() == id)
        {
            std::cout << "Found a match!\n";
            return f;
        }
    }

    return nullptr;
}