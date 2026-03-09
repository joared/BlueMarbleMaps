#ifndef STANDARDLAYER
#define STANDARDLAYER

#include "BlueMarbleMaps/Core//Layer/Layer.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"
#include "BlueMarbleMaps/System/Thread.h"

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
            virtual FeatureEnumeratorPtr prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
            virtual void update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery) override final;
            virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override final;
            virtual void flushCache() override final;
        private:
            // TODO: possibly make these part of base "Layer"
            IdCollectionPtr getFeatureIds(const CrsPtr& crs, const FeatureQuery& featureQuery);
            FeatureCollectionPtr getFeatures(const CrsPtr& crs, const IdCollectionPtr& ids);

            void createDefaultVisualizers();

            std::vector<VisualizerPtr> m_visualizers;
            std::vector<VisualizerPtr> m_hoverVisualizers;
            std::vector<VisualizerPtr> m_selectionVisualizers;
            std::vector<EffectPtr> m_effects;

            std::vector<DataSetPtr> m_dataSets;

            FIFOCachePtr            m_cache;
            bool                    m_readAsync;
            std::mutex              m_mutex;
            FeatureQuery            m_query;
            System::ThreadPool      m_threadPool;

            FeatureEnumeratorPtr    m_queriedFeatures;


    };
    typedef std::shared_ptr<StandardLayer> StandardLayerPtr;

}

#endif /* STANDARDLAYER */
