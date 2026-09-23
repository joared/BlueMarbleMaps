#ifndef MAP
#define MAP

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Camera/Camera.h"
#include "BlueMarbleMaps/Core/Camera/ICameraController.h"
#include "BlueMarbleMaps/Core/Animation.h"
#include "BlueMarbleMaps/Core/Drawable.h"
#include "BlueMarbleMaps/Core/Layer/Layer.h"
#include "BlueMarbleMaps/Core/PresentationObject.h"
#include "BlueMarbleMaps/Core/ResourceObject.h"
#include "BlueMarbleMaps/CoordinateSystem/Crs.h"
#include "BlueMarbleMaps/Event/Signal.h"
#include "BlueMarbleMaps/CoordinateSystem/SurfaceModel.h"
#include "BlueMarbleMaps/Core/Reflection/IReflectedObject.h"

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
        , public Reflection::IReflectedObject
    {
        BMM_REFLECTED(Map, IReflectedObject)

        static void reflect(Reflection::TypeBuilder<Map>& t)
        {
            t.property("showDebugInfo", &Map::m_showDebugInfo).displayName("Show debug info");
            t.property("scale", &Map::scale).displayName("Scale");
            t.property("invertedScale", &Map::invertedScale).displayName("Inverted scale");
            t.operation("panTo", [](Map* map) { map->panTo({0,0}); map->update(); });
            //t.operation<void, double, double, double>("panTo2", [](Map* map, double x, double y, double z) { map->panTo({x,y,z}); });
            t.operation("rotateTo", &Map::rotateTo);
            std::function<void(Map*, double, double, double)> test =
            [](Map* map, double x, double y, double z)
            {
                map->panTo({x, y, z});
            };
            t.operation("panTo3", test);
            // t.operation("pixelToScreen", &Map::screenToPixel);
            //t.operation("toggleDebug", &Map::m_showDebugInfo); // Why does this compile???
            t.operation("toggleDebug", [](Map* map) { map->showDebugInfo() = !map->showDebugInfo(); });

        }

        public:
            

            Map();
            Map(const Map&) = delete;
            Map& operator=(const Map&) = delete;
            Map(Map&&) = delete;
            Map& operator=(Map&&) = delete;

            bool update(bool forceUpdate=false);
 
            // Camera properties
            const CameraUniquePtr& camera() const;
            void setDrawableFromCamera(const CameraUniquePtr& camera);
            double invertedScale() const;
            double scale() const;
            // These could be useful to add back later
            // void center(const Point& center);
            // void scale(double scale);
            // const Point& center() const;
            // void invertedScale(double invScale);
            // double rotation() const;
            // void rotation(double rotation);
            // double width() const;
            // void width(double newWidth);
            // double height() const;
            // Rectangle area() const;

            const CrsPtr& crs() const { return m_crs; }
            void crs(const CrsPtr& crs);
            SurfaceModelPtr surfaceModel() { return m_surfaceModel; };
            void setSurfaceModel(const SurfaceModelPtr& model);
            
            // Camera controller
            template <typename T = ICameraController>
            T* cameraController() const { return dynamic_cast<T*>(m_cameraController.get()); }
            void setCameraController(ICameraControllerUniquePtr controller);
            // Primitive camera controller options, for simple panning and zooming.
            // Forwarded to the internal camera controller, if one is set, and if the
            // controller implements the ICameraNavigator interface.
            void panTo(const Point& target);
            void rotateTo(double angle);
            void zoomTo(const Rectangle& bounds);

            Point pixelToScreen(const Point& pixel) const;
            Point pixelToScreen(int px, int py) const;
            Point screenToPixel(const Point& screen) const;
            Point screenToPixel(double x, double y) const;
            Point screenCenter() const;
            Point screenToMap(const Point& screenPos) const;
            Point screenToMap(double x, double y) const;
            Point screenToMapAtHeight(const Point& screenPos, double heightMeters) const;
            Point mapToScreen(const Point& point) const;
            Ray screenToViewRay(double x, double y) const;
            Ray screenToMapRay(double x, double y) const;
            void screenToNDC(double x, double y, double& ndcX, double& ndcY) const;
            void ndcToScreen(double ndcx, double ndcy, double& x, double& y) const;
            std::vector<Point> screenToMap(const std::vector<Point>& points) const;
            std::vector<Point> mapToScreen(const std::vector<Point>& points) const;
            std::vector<Point> lngLatToMap(const std::vector<Point>& points) const;
            Rectangle screenToMap(const Rectangle& rect) const;
            Rectangle mapToScreen(const Rectangle& rect) const;

            const Point mapToLngLat(const Point& mapPoint, bool normalize=true) const;
            const Point lngLatToMap(const Point& lngLat) const;

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
            void deSelect(const Id& id);
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

            // For drawing screen-anchored 3D widgets (e.g. a north arrow) from an onCustomDraw handler.
            // Flushes pending batches and sets the drawable's matrices so that local coordinates are
            // ordinary screen coordinates (pixels, x right, y DOWN, z into the screen) with the origin
            // at `screenPos`, rotated as the current camera sees the world, and with the perspective
            // centred on `screenPos` rather than on the screen centre. With an unrotated camera this
            // is the plain screen frame: north is (0,-1,0) and up out of the map is (0,0,-1).
            // The next handler starts again from the plain screen transform.
            void setScreenWidgetTransform(const Point& screenPos);

            void resize(int width, int height);

            void flushCache();

            bool& showDebugInfo() { return m_showDebugInfo; }
            void onAttachedToMapControl(MapControlPtr mapControl) { m_mapControl = mapControl; };
            void onDetachedFromMapControl() { m_mapControl = nullptr; };

            struct MapEvents
            {
                // Update events
                Signal<Map&> onCameraChanged; // This event needs refinement, dont use
                Signal<Map&> onUpdating;
                Signal<Map&> onDrawBackground;
                Signal<Map&> onCustomDraw;
                Signal<Map&> onUpdated;
                Signal<Map&> onIdle;          // This event needs refinement, dont use

                // State events
                Signal<Map&, int, int>                                       onSizeChanged;           // width, height
                Signal<Map&, const CrsPtr&, const CrsPtr&>                   onCrsChanged;            // oldSurface, newSurface
                Signal<Map&, const SurfaceModelPtr&, const SurfaceModelPtr&> onSurfaceModelChanged;   // oldCrs, newCrs
                Signal<Map&, const Id&>                                      onHoverChanged;          // hoveredId
                Signal<Map&, const IdCollectionPtr&>                         onSelectionChanged;      // selectedIds

            } events;

            // Debug options
            bool renderingEnabled() { return m_renderingEnabled; };
            void renderingEnabled(bool enabled);

        private:
            void updateUpdateAttributes(int64_t timeStampMs);
            void beforeRender();
            void calculateAppropriateNearFar(double& near, double& far) const;
            void renderLayers();
            FeatureQuery produceUpdateQuery();
            FeatureQuery produceUpdateQuery(const Rectangle& mapArea);
            void renderLayer(const LayerPtr& layer, const FeatureQuery& featureQuery);
            void afterRender();

            void drawTestElevationMesh();

            void drawDebugInfo(int elapsedMs);

            MapControlPtr m_mapControl;
            DrawablePtr m_drawable;
            DrawablePtr m_offscreenDrawable;

            CrsPtr          m_crs;
            SurfaceModelPtr m_surfaceModel;

            bool m_updateRequired;
            bool m_updateEnabled;
            bool m_quickUpdateEnabled;

            CameraUniquePtr             m_camera;
            ICameraControllerUniquePtr  m_cameraController;
            ICameraNavigator*           m_cameraNavigator;
            int64_t                     m_lastUpdateTimeStamp;

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
