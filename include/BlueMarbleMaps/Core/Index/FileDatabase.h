#ifndef BLUEMARBLE_FILEDATABASE
#define BLUEMARBLE_FILEDATABASE

#include "IFeatureDataBase.h"
#include "BlueMarbleMaps/System/File.h"

namespace BlueMarble
{
    
    class FileDatabase : public IFeatureDataBase
    {
    public:
        struct FeatureRecord
        {
            int64_t lineOffset;
        };

        FileDatabase();
        virtual FeaturePtr getFeature(const FeatureId& id) override final;
        virtual FeatureCollectionPtr getFeatures(const FeatureIdCollectionPtr& ids) override final;
        virtual void getFeatures(const FeatureIdCollectionPtr& ids, FeatureCollectionPtr& featuresOut) override final;
        virtual FeatureCollectionPtr getAllFeatures() override final;
        virtual void removeFeature(const FeatureId& id) override final;
        virtual size_t size() const override final;

        virtual void save(const std::string& path) const override final;
        virtual bool load(const std::string& path) override final;
        virtual bool build(const FeatureCollectionPtr& features, const std::string& path) override final;
    private:
        void verifyLoaded() const;

        std::string m_filePath;
        File        m_file;
        std::map<FeatureId, FeatureRecord> m_index;
        bool m_isLoaded;

    };
}

#endif /* BLUEMARBLE_FILEDATABASE */
