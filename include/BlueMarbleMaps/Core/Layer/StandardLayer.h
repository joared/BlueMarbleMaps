#ifndef BLUEMARBLE_STANDARDLAYER
#define BLUEMARBLE_STANDARDLAYER

#include "BlueMarbleMaps/Core//Layer/Layer.h"
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
            StandardLayer(bool createDefaultVisualizers = true);
            ~StandardLayer();
            void asyncRead(bool async);
            bool asyncRead();
            void addDataSet(const DataSetPtr& dataSet);

            std::vector<VisualizerPtr>& visualizers() { return m_visualizers; }
            std::vector<VisualizerPtr>& hoverVisualizers() { return m_hoverVisualizers; }
            std::vector<VisualizerPtr>& selectionVisualizers() { return m_selectionVisualizers; }

            std::vector<EffectPtr>& effects() { return m_effects; }

            virtual void hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects) override final;
            virtual void prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
            virtual void update(const MapPtr& map) override final;
            virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override final;
            virtual void flushCache() override final;
        private:
            IdCollectionPtr getFeatureIds(const CrsPtr& crs, const FeatureQuery& featureQuery); // TODO: possibly make this part of base "Layer"
            void createDefaultVisualizers();

            void backgroundReadingThread();
            void startbackgroundReadingThread();
            void stopBackgroundReadingThread();

            std::vector<VisualizerPtr> m_visualizers;
            std::vector<VisualizerPtr> m_hoverVisualizers;
            std::vector<VisualizerPtr> m_selectionVisualizers;
            std::vector<EffectPtr> m_effects;

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
            std::function<void()>   m_job;

            FeatureEnumeratorPtr    m_queriedFeatures;


    };
    typedef std::shared_ptr<StandardLayer> StandardLayerPtr;

}

#endif /* BLUEMARBLE_STANDARDLAYER */
