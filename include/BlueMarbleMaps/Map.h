#ifndef MAP
#define MAP

#include "Core.h"
#include "MapConstraints.h"
#include "Animation.h"
#include "Drawable.h"
#include "Layer.h"
#include "PresentationObject.h"
#include "EngineObject.h"
#include "CoordinateSystem/Crs.h"

#include <map>
#include <functional>


namespace BlueMarble
{
    // Forward declarations
    class MapControl;
    class MapEventHandler;

    enum class SelectMode
    {
        Add,
        Replace
    };

    class MapEventHandler
    {
        public:
            virtual void OnAreaChanged(Map& map) = 0;
            virtual void OnUpdating(Map& map) = 0;
            virtual void OnCustomDraw(Map& map) = 0;
            virtual void OnUpdated(Map& map) = 0;
    };

    class MapEventPublisher
    {
        public:
            MapEventPublisher();
            void sendOnAreaChanged(Map& map);
            void sendOnUpdating(Map& map);
            void sendOnCustomDraw(Map& map);
            void sendOnUpdated(Map& map);

            void addMapEventHandler(MapEventHandler* handler);
            void removeMapEventHandler(MapEventHandler* handler);
        private:
            std::vector<MapEventHandler*> m_eventHandlers;
    };

    class Map 
        : public EngineObject
        , public MapEventPublisher
    {
        class MapCommand
        {
            public:
                inline MapCommand(Map& map, const std::function<void()>& action)
                    : m_map(map)
                    , m_action(action)
                {

                }
                inline void execute()
                {
                    m_from = m_map.area();
                    m_action();
                }

                inline void revert()
                {
                    m_map.zoomToArea(m_from, true);
                }

            private:
                Map& m_map;
                const std::function<void()>& m_action;
                Rectangle m_from;
        };
        public:
            Map();
            bool update(bool forceUpdate=false);

            void renderLayers();
 
            const Point& center() const;
            void center(const Point& center);
            void scale(double scale);
            double scale() const;
            double invertedScale() const;
            void invertedScale(double invScale);
            double rotation() const;
            void rotation(double rotation);
            double width() const;
            double height() const;
            Rectangle area() const;
            MapConstraints& mapConstraints() { return m_constraints; };
            const CrsPtr& getCrs() { return m_crs; }
            void setCrs(const CrsPtr& crs) { m_crs = crs; }
            // Operations
            void panBy(const Point& deltaScreen, bool animate=false);
            void panTo(const Point& mapPoint, bool animate=false);
            void zoomTo(const Point& mapPoint, double newScale, bool animate=false);
            void zoomOn(const Point& mapPoint, double zoomFactor, bool animate=false);
            void zoomToArea(const Rectangle& bounds, bool animate=false);
            void zoomToMinArea(const Rectangle& bounds, bool animate=false);

            Point screenToMap(const Point& screenPos) const;
            Point screenToMap(double x, double y) const;
            Point mapToScreen(const Point& point) const;
            std::vector<Point> screenToMap(const std::vector<Point>& points) const;
            std::vector<Point> mapToScreen(const std::vector<Point>& points) const;
            Rectangle screenToMap(const Rectangle& rect) const;
            Rectangle mapToScreen(const Rectangle& rect) const;
            const Point screenToLngLat(const Point& screenPoint);           // Temporary test, should be removed
            const Point mapToLngLat(const Point& mapPoint, bool normalize=true); // Temporary test, should be removed
            const Point lngLatToMap(const Point& lngLat);                   // Temporary test, should be removed
            const Point lngLatToScreen(const Point& lngLat);                // Temporary test, should be removed
            std::vector<Point> lngLatToScreen(const std::vector<Point>& points);   // Temporary test, should be removed
            Rectangle lngLatToMap(const Rectangle& rect);                   // Temporary test, should be removed
            Rectangle mapToLngLat(const Rectangle& rect);                   // Temporary test, should be removed
            Rectangle lngLatToScreen(const Rectangle& rect);                   // Temporary test, should be removed

            Point screenCenter() const;

            void startAnimation(AnimationPtr animation);
            void stopAnimation();

            // Properties
            bool updateEnabled() const { return m_updateEnabled; };
            void updateEnabled(bool enabled) { m_updateEnabled = enabled; };
            bool quickUpdateEnabled() const { return m_quickUpdateEnabled; }
            void quickUpdateEnabled(bool enabled) { m_quickUpdateEnabled = enabled; }
            const Attributes& updateAttributes() const { return m_updateAttributes; };
            Attributes& updateAttributes() { return m_updateAttributes; };

            void addLayer(Layer* layer);
            std::vector<Layer*>& layers();
            
            FeaturePtr getFeature(const Id& id);
            void getFeatures(const Attributes& attributes, std::vector<FeaturePtr>& features);
            std::vector<FeaturePtr> featuresAt(int X, int Y, double pointerRadius);
            void featuresInside(const Rectangle& bounds, FeatureCollection& out);
            std::vector<PresentationObject>& presentationObjects() { return m_presentationObjects; } // TODO: should be exposed like this
            std::vector<PresentationObject> hitTest(int x, int y, double pointerRadius);
            std::vector<PresentationObject> hitTest(const Rectangle& bounds);
            void select(FeaturePtr feature, SelectMode mode=SelectMode::Replace);
            const std::vector<Id>& selected() { return m_selectedFeatures; }
            void deSelect(FeaturePtr feature);
            void deSelectAll();
            bool isSelected(const Id& id);
            bool isSelected(FeaturePtr feature);
            void hover(const Id& id);
            void hover(FeaturePtr feature);
            void hover(const std::vector<Id>& ids);
            void hover(const std::vector<FeaturePtr>& features);
            std::vector<Id> hovered() { return m_hoveredFeatures; };
            bool isHovered(const Id& id);
            bool isHovered(FeaturePtr feature);

            DrawablePtr drawable();
            void drawable(const DrawablePtr& drawable);

            // Update flags
            bool centerChanged() { return m_centerChanged; }
            bool scaleChanged() { return m_scaleChanged; }
            bool rotationrChanged() { return m_rotationChanged; }

            void startInitialAnimation();
            void doCommand(const std::function<void()>& action);
            bool undoCommand();
            bool& showDebugInfo() { return m_showDebugInfo; }
            void onAttachedToMapControl(MapControl* mapControl) { m_mapControl = mapControl; };
            void onDetachedFromMapControl() { m_mapControl = nullptr; };
        private:
            void updateUpdateAttributes(int64_t timeStampMs);
            void beforeRender();
            void afterRender();
            
            void resetUpdateFlags();

            void drawDebugInfo(int elapsedMs);

            MapControl* m_mapControl;
            DrawablePtr m_drawable;
            
            Point m_center;
            double m_scale;
            double m_rotation;

            CrsPtr m_crs;

            MapConstraints m_constraints;

            bool m_updateRequired;
            bool m_updateEnabled;
            bool m_quickUpdateEnabled;

            // Update flags
            bool m_centerChanged;
            bool m_scaleChanged;
            bool m_rotationChanged;

            AnimationPtr    m_animation;
            int m_animationStartTimeStamp;
            Attributes m_updateAttributes;

            std::vector<Layer*> m_layers;
            std::vector<PresentationObject> m_presentationObjects;
            std::vector<Id>                 m_selectedFeatures;
            std::vector<Id>                 m_hoveredFeatures;

            MapCommand*                     m_commmand;

            bool m_showDebugInfo;
            bool m_isUpdating; // Not allowed to call update() within an update() call
    };

    typedef std::shared_ptr<Map> MapPtr;

}

#endif /* MAP */
