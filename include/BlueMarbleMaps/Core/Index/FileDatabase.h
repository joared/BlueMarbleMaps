#ifndef BLUEMARBLE_FILEDATABASE
#define BLUEMARBLE_FILEDATABASE

#include "IFeatureDataBase.h"

namespace BlueMarble
{
    class FileDatabase : public IFeatureDataBase
    {
    public:
        FileDatabase();
        virtual void addFeature(const FeaturePtr& feature) override final;
        virtual FeaturePtr getFeature(const FeatureId& id) const override final;
        virtual FeatureCollectionPtr getFeatures(const FeatureIdCollectionPtr& ids) const override final;
        virtual void getFeatures(const FeatureIdCollectionPtr& ids, FeatureCollectionPtr& featuresOut) const override final;
        virtual FeatureCollectionPtr getAllFeatures() const override final;
        virtual void removeFeature(const FeatureId& id) override final;
        virtual size_t size() const override final;

        virtual void save(const std::string& path) const override final;
        virtual bool load(const std::string& path) override final;
    private:
        int m_entryCount;
        std::string m_filePath;
    };
}

#endif /* BLUEMARBLE_FILEDATABASE */
