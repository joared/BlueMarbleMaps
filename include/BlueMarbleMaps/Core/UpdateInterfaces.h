#ifndef BLUEMARBLE_UPDATEINTERFACES
#define BLUEMARBLE_UPDATEINTERFACES

#include "Feature.h"

namespace BlueMarble
{
    class Map;              // Forward declaration.
    class FeatureHandler;  // Forward declaration.

    // New stuff
    class FeatureQuery
    {
        public:
            FeatureQuery()
            {

            }
            const Rectangle& area() const { return m_area; }
            void area(const Rectangle& area) { m_area = area; }
            double scale() const { return m_scale; }
            void scale(double scale) { m_scale = scale; }
            Attributes* updateAttributes() const { return m_updateAttributes; }
            void updateAttributes(Attributes* attr) { m_updateAttributes = attr; }
            bool quickUpdate() const { return m_quickUpdate; }
            void quickUpdate(bool quickpdate) { m_quickUpdate = quickpdate; }

        private:
            Rectangle   m_area = Rectangle::infinite();
            double      m_scale = 1.0;
            Attributes* m_updateAttributes = nullptr;
            bool        m_quickUpdate = false;
    };

    class FeatureEnumerator;
    typedef std::shared_ptr<FeatureEnumerator> FeatureEnumeratorPtr;
    class FeatureEnumerator
    {
        public:
            FeatureEnumerator();
            void addEnumerator(const FeatureEnumeratorPtr& enumerator) { m_subEnumerators.push_back(enumerator); }
            const FeaturePtr& current() const;
            bool moveNext();
            void reset();
            void add(const FeaturePtr& feature);
            void setFeatures(const FeatureCollectionPtr& features);
            int size();
        private:
            int m_iteratorIndex;
            int m_iterationIndex;
            FeatureCollectionPtr m_features;
            std::vector<FeatureEnumeratorPtr> m_subEnumerators;
    };

    // Downstream
    // Interface for layers, operators and data sets that handles update requests in the operator chain
    class IUpdateHandler
    {
        public:
            // New stuff
            // virtual const FeatureEnumeratorPtr& getFeatures(const FeatureQuery& query) = 0;
            // Old
            virtual void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) = 0;
            virtual void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) = 0;
            virtual FeaturePtr onGetFeatureRequest(const Id& id) = 0;
            virtual ~IUpdateHandler() = default;

    };

    // Upstream
    // Interface for layers, operators and data sets that handles features as a results of and update request
    class FeatureHandler
    {
        public:
            FeatureHandler();
            // TODO: we send "back" a reference to the map since layers needs it to render
            virtual void onFeatureInput(Map &map, const std::vector<FeaturePtr>& features) = 0;
            virtual ~FeatureHandler() = default;
            void addUpdateHandler(IUpdateHandler* handler);
        protected:
            void sendUpdateRequest(Map& map, const Rectangle& updateArea);
            void sendGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features);
            FeaturePtr sendGetFeatureRequest(const Id& id);
        private:
            std::vector<IUpdateHandler*> m_updateHandlers;
    };
}

#endif /* BLUEMARBLE_UPDATEINTERFACES */
