#ifndef BLUEMARBLE_FILEDATABASE
#define BLUEMARBLE_FILEDATABASE

#include "IFeatureDataBase.h"
#include "IPersistable.h"
#include "BlueMarbleMaps/System/File.h"

namespace BlueMarble
{
    
    class FileDatabase : public IFeatureDataBase, public IPersistable
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
        virtual bool build(const FeatureCollectionPtr& features) override final;
        
        virtual std::string persistanceId() const { return "file"; }
        virtual void save(const PersistanceContext& path) const override final;
        virtual bool load(const PersistanceContext& path) override final;
        
    private:
        void verifyLoaded() const;

        mutable FeatureCollectionPtr m_stage;

        std::string m_filePath;
        std::unique_ptr<File>        m_file;
        std::map<FeatureId, FeatureRecord> m_index;
        bool m_isLoaded;

    };
}

#endif /* BLUEMARBLE_FILEDATABASE */
