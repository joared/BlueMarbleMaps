#ifndef MEMORYDATASET
#define MEMORYDATASET

#include "DataSet.h"
#include "BlueMarbleMaps/Core/FeatureAnimation.h"

namespace BlueMarble
{
    class IFeatureEventListener
    {
        public:
            virtual void onFeatureCreated(const FeaturePtr& feature) = 0;
            virtual void onFeatureUpdated(const FeaturePtr& feature) = 0;
            virtual void onFeatureDeleted(const Id& id) = 0;
    };

    class FeatureEventPublisher
    {
        public:
            FeatureEventPublisher(bool eventsEnabled);
            void addFeatureEventListener(IFeatureEventListener* listener, const Id& id);
            void removeFeatureEventListener(IFeatureEventListener* listener, const Id& id);
            bool featureEventsEnabled();
            void featureEventsEnabled(bool enabled);
        protected:
            void sendOnFeatureCreated(const FeaturePtr& feature) const;
            void sendOnFeatureUpdated(const FeaturePtr& feature) const;
            void sendOnFeatureDeleted(const Id& id) const;
        private:
            std::map<Id, std::vector<IFeatureEventListener*>> m_listeners;
            bool m_eventsEnabled;
    };

    class MemoryDataSet 
        : public DataSet
        , public FeatureEventPublisher
    {
        public:
            MemoryDataSet();
            void addFeature(FeaturePtr feature);
            void removeFeature(const Id& id);
            void clear();
            // void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            // void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            // FeaturePtr onGetFeatureRequest(const Id& id) override final;

            virtual FeatureEnumeratorPtr getFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeaturePtr getFeature(const Id& id) override final;

            void startFeatureAnimation(FeaturePtr feature);
            void startFeatureAnimation(FeaturePtr feature, const Point& from, const Point& to);
            void triggerFeatureUpdated(const FeaturePtr& id);
        protected:
            void init() override final;
        private:
            FeatureCollection m_features;
            std::map<Id, FeatureAnimationPtr> m_idToFeatureAnimation;
    };
    typedef std::shared_ptr<MemoryDataSet> MemoryDataSetPtr;
}

#endif /* MEMORYDATASET */
