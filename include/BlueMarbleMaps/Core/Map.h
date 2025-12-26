#ifndef MAP
#define MAP

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Camera.h"
#include "BlueMarbleMaps/Core/MapConstraints.h"
#include "BlueMarbleMaps/Core/Animation.h"
#include "BlueMarbleMaps/Core/Drawable.h"
#include "BlueMarbleMaps/Core/Layer/Layer.h"
#include "BlueMarbleMaps/Core/PresentationObject.h"
#include "BlueMarbleMaps/Core/ResourceObject.h"
#include "BlueMarbleMaps/CoordinateSystem/Crs.h"
#include "BlueMarbleMaps/Event/Signal.h"

#include <map>
#include <functional>


namespace BlueMarble
{
    // Forward declarations
    class MapControl;
    typedef std::shared_ptr<MapControl> MapControlPtr;

    enum class SelectMode
    {
        Add,
        Replace
    };

    class Map;
    typedef std::shared_ptr<Map> MapPtr;
    class Map 
        : public std::enable_shared_from_this<Map>
        , public ResourceObject
    {
        public:
            Map();
            Map(const Map&) = delete;
            Map& operator=(const Map&) = delete;
            Map(Map&&) = delete;
            Map& operator=(Map&&) = delete;

            bool update(bool forceUpdate=false);
 
            // TODO: move to camera/camera controller
            // Camera properties
            CameraPtr camera() { return m_camera; }
            void setDrawableFromCamera(const CameraPtr& camera);
            const Point& center() const;
            void center(const Point& center);
            void scale(double scale);
            double scale() const;
            double invertedScale() const;
            void invertedScale(double invScale);
            double rotation() const;
            void rotation(double rotation);
            double tilt() const;
            void tilt(double tilt);
            double width() const;
            void width(double newWidth);
            double height() const;
            Rectangle area() const;
            MapConstraints& mapConstraints() { return m_constraints; };
            const CrsPtr& crs() { return m_crs; }
            void crs(const CrsPtr& crs);
            
            // TODO: move to cameracontroller class
            // Camera controlling
            void panBy(const Point& deltaScreen, bool animate=false);
            void panTo(const Point& mapPoint, bool animate=false);
            void zoomTo(const Point& mapPoint, double newScale, bool animate=false);
            void zoomOn(const Point& mapPoint, double zoomFactor, bool animate=false);
            void zoomToArea(const Rectangle& bounds, bool animate=false);
            void zoomToMinArea(const Rectangle& bounds, bool animate=false);

            Point screenCenter() const;
            Point pixelToScreen(int px, int py) const;
            Point screenToPixel(double x, double y) const;
            Point screenToMap(const Point& screenPos) const;
            Point screenToMap(double x, double y) const;
            Point mapToScreen(const Point& point) const;
            Point screenToViewRay(double x, double y) const;
            Point screenToMapRay(double x, double y) const;
            void screenToNDC(double x, double y, double& ndcX, double& ndcY) const;
            void ndcToScreen(double ndcx, double ndcy, double& x, double& y) const;
            std::vector<Point> screenToMap(const std::vector<Point>& points) const;
            std::vector<Point> mapToScreen(const std::vector<Point>& points) const;
            std::vector<Point> lngLatToMap(const std::vector<Point>& points) const;
            Rectangle screenToMap(const Rectangle& rect) const;
            Rectangle mapToScreen(const Rectangle& rect) const;
            const Point screenToLngLat(const Point& screenPoint);           // Temporary test, should be removed
            const Point mapToLngLat(const Point& mapPoint, bool normalize=true); // Temporary test, should be removed
            const Point lngLatToMap(const Point& lngLat) const;                   // Temporary test, should be removed
            const Point lngLatToScreen(const Point& lngLat);                // Temporary test, should be removed
            std::vector<Point> lngLatToScreen(const std::vector<Point>& points);   // Temporary test, should be removed
            Rectangle lngLatToMap(const Rectangle& rect);                   // Temporary test, should be removed
            Rectangle mapToLngLat(const Rectangle& rect);                   // Temporary test, should be removed
            Rectangle lngLatToScreen(const Rectangle& rect);                   // Temporary test, should be removed

            void startAnimation(AnimationPtr animation);
            void stopAnimation();

            // Properties
            bool updateEnabled() const { return m_updateEnabled; };
            void updateEnabled(bool enabled) { m_updateEnabled = enabled; };
            bool quickUpdateEnabled() const { return m_quickUpdateEnabled; }
            void quickUpdateEnabled(bool enabled) { m_quickUpdateEnabled = enabled; }
            const Attributes& updateAttributes() const { return m_updateAttributes; };
            Attributes& updateAttributes() { return m_updateAttributes; };

            void addLayer(const LayerPtr& layer);
            std::vector<LayerPtr>& layers();
            
            //void getFeatures(const Attributes& attributes, std::vector<FeaturePtr>& features);
            std::vector<FeaturePtr> featuresAt(int X, int Y, double pointerRadius);
            void featuresInside(const Rectangle& bounds, FeatureCollection& out);
            std::vector<PresentationObject>& presentationObjects() { return m_presentationObjects; } // TODO: should be exposed like this
            const std::vector<PresentationObject>& hitTest(int x, int y, double pointerRadius);
            const std::vector<PresentationObject>& hitTest(const Rectangle& bounds);
            void select(FeaturePtr feature, SelectMode mode=SelectMode::Replace);
            void select(const PresentationObject& presentationObject);
            const std::vector<PresentationObject>& selectedPresentationObjects();
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
            void resize(int width, int height);

            void flushCache();

            // Update flags
            bool centerChanged() { return m_centerChanged; }
            bool scaleChanged() { return m_scaleChanged; }
            bool rotationrChanged() { return m_rotationChanged; }

            void startInitialAnimation();
            void doCommand(const std::function<void()>& action);
            bool undoCommand();
            bool& showDebugInfo() { return m_showDebugInfo; }
            void onAttachedToMapControl(MapControlPtr mapControl) { m_mapControl = mapControl; };
            void onDetachedFromMapControl() { m_mapControl = nullptr; };

            struct MapEvents
            {
                // Update events
                Signal<Map&> onAreaChanged;
                Signal<Map&> onUpdating;
                Signal<Map&> onCustomDraw;
                Signal<Map&> onUpdated;
                Signal<Map&> onIdle;

                // State events
                Signal<Map&, const Id&> onHoverChanged;
                Signal<Map&, const IdCollectionPtr&> onSelectionChanged;

            } events;

            // Debug options
            bool renderingEnabled() { return m_renderingEnabled; };
            void renderingEnabled(bool enabled);

        private:
            void setCamera();
            void updateUpdateAttributes(int64_t timeStampMs);
            void beforeRender();
            void renderLayers();
            FeatureQuery produceUpdateQuery();
            void renderLayer(const LayerPtr& layer, const FeatureQuery& featureQuery);
            void afterRender();
            
            void resetUpdateFlags();

            void drawDebugInfo(int elapsedMs);

            MapControlPtr m_mapControl;
            DrawablePtr m_drawable;
            
            Point m_center;
            double m_scale;
            double m_rotation;
            double m_tilt;

            CrsPtr m_crs;

            MapConstraints m_constraints;

            bool m_updateRequired;
            bool m_updateEnabled;
            bool m_quickUpdateEnabled;

            // Update flags
            bool m_centerChanged;
            bool m_scaleChanged;
            bool m_rotationChanged;
            mutable CameraPtr m_camera;

            AnimationPtr    m_animation;
            int m_animationStartTimeStamp;
            Attributes m_updateAttributes;

            std::vector<LayerPtr> m_layers;
            std::vector<PresentationObject> m_presentationObjects;
            std::vector<Id>                 m_selectedFeatures;
            std::vector<Id>                 m_hoveredFeatures;
            std::vector<PresentationObject> m_selectedPresentationObjects;

            bool m_showDebugInfo;
            bool m_isUpdating; // Not allowed to call update() within an update() call

            // Debugging
            bool m_renderingEnabled;
    };

}

#endif /* MAP */
