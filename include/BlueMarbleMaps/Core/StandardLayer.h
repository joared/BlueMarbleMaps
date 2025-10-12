#ifndef BLUEMARBLE_STANDARDLAYER
#define BLUEMARBLE_STANDARDLAYER

#include "BlueMarbleMaps/Core/Layer.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"

#include <thread>
#include <condition_variable>


namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    typedef std::shared_ptr<Map> MapPtr;
    
    class StandardLayer : public Layer
    {
        public:
            StandardLayer(bool createdefaultVisualizers = true);
            ~StandardLayer();
            void asyncRead(bool async);
            bool asyncRead();
            void addDataSet(const DataSetPtr& dataSet);
            virtual FeatureEnumeratorPtr update(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
            virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override final;
        private:
            void backgroundReadingThread();
            void startbackgroundReadingThread();
            void stopBackgroundReadingThread();

            std::vector<DataSetPtr> m_dataSets;
            FIFOCachePtr            m_cache;
            bool                    m_readAsync;
            std::thread             m_readingThread;
            std::condition_variable m_cond;
            std::mutex              m_mutex;
            bool                    m_doRead;
            bool                    m_stop;
            FeatureQuery            m_query;
            CrsPtr                  m_crs;


    };
    typedef std::shared_ptr<StandardLayer> StandardLayerPtr;

}

#endif /* BLUEMARBLE_STANDARDLAYER */
