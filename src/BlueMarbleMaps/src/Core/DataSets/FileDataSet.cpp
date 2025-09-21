#include "BlueMarbleMaps/Core/DataSets/FileDataSet.h"
#include "BlueMarbleMaps/Core/Index/QuadTreeIndex.h"
#include "BlueMarbleMaps/Core/Index/FileDatabase.h"
#include "BlueMarbleMaps/Core/Index/MemoryDatabase.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"

using namespace BlueMarble;

AbstractFileDataSet::AbstractFileDataSet(const std::string& filePath)
    : DataSet() 
    , m_filePath(filePath)
    , m_featureStore()
    , m_progress(0)
{
    auto db = std::make_unique<MemoryDatabase>();//std::make_unique<FileDatabase>(); // TODO: change to FileDataSet
    auto index = std::make_unique<QuadTreeIndex>(Rectangle(-180, -90, 180, 90), 0.05);
    auto cache = std::make_shared<FIFOCache>();
    m_featureStore = std::make_unique<FeatureStore>(dataSetId(), std::move(db), std::move(index), cache);
}

FeatureEnumeratorPtr AbstractFileDataSet::getFeatures(const FeatureQuery &featureQuery)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();

    if (!isInitialized()) // TODO: move to base class
    {
        return enumerator;
    }

    auto features = m_featureStore->query(featureQuery.area());
    enumerator->setFeatures(features);

    return enumerator;
}

void AbstractFileDataSet::init()
{
    std::string indexPath = "spatial_index/test_index";
    if (!m_featureStore->load(indexPath))
    {
        read(m_filePath);
        for (auto f : m_features)
        {
            m_featureStore->addFeature(f);
        }
        m_featureStore->save(indexPath);
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