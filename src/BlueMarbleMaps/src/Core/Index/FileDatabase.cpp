#include "BlueMarbleMaps/Core/Index/FileDatabase.h"
#include "BlueMarbleMaps/System/File.h"

#include <fstream>

using namespace BlueMarble;

FileDatabase::FileDatabase()
    : m_entryCount(0)
    , m_filePath()
{
}

void FileDatabase::addFeature(const FeaturePtr& feature)
{

}

FeaturePtr FileDatabase::getFeature(const FeatureId& id) const
{
    return FeaturePtr();
}

FeatureCollectionPtr FileDatabase::getFeatures(const FeatureIdCollectionPtr& ids) const
{
    return FeatureCollectionPtr();
}

void BlueMarble::FileDatabase::getFeatures(const FeatureIdCollectionPtr &ids, FeatureCollectionPtr &featuresOut) const
{
}

FeatureCollectionPtr FileDatabase::getAllFeatures() const
{
    return FeatureCollectionPtr();
}

void FileDatabase::removeFeature(const FeatureId& id)
{
}

size_t FileDatabase::size() const
{
    return m_entryCount;
}

void FileDatabase::save(const std::string& path) const
{
    
}

bool FileDatabase::load(const std::string& path)
{
    m_filePath = path;

    //File file(path);

    std::ifstream file(path);
    // Check if the file is successfully opened 
    if (!file.is_open()) 
    { 
        std::cerr << "File::readLines() Error opening the file!\n";
    }

    std::string line;
    while (getline(file, line))
    {
        
    }

    file.close();

    return false;
}
