#ifndef BLUEMARBLE_MAP
#define BLUEMARBLE_MAP

#include "Core.h"
#include "MapConstraints.h"
#include "Animation.h"
#include "Drawable.h"
#include "Layer.h"
#include "PresentationObject.h"

#include <CImg.h>

#include <map>

namespace BlueMarble
{
    class MapEventHandler; // Forward declaration

    enum class SelectMode
    {
        Add,
        Replace
    };

    class Map
    {
        public:
            Map(cimg_library::CImgDisplay& disp);
            bool update(bool forceUpdate=false);

            void render();              // 1
            void renderWithOverviews(); // 2
            void renderLayers();        // 3

 
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

            // Operations
            void panBy(const Point& deltaScreen, bool animate=false);
            void zoomOn(const Point& mapPoint, double zoomFactor, bool animate=false);
            void zoomToArea(const Rectangle& bounds, bool animate=false);

            Point screenToMap(const Point& screenPos) const;
            Point screenToMap(double x, double y) const;
            Point mapToScreen(const Point& point) const;
            std::vector<Point> screenToMap(const std::vector<Point>& points) const;
            std::vector<Point> mapToScreen(const std::vector<Point>& points) const;
            Rectangle screenToMap(const Rectangle& rect) const;
            Rectangle mapToScreen(const Rectangle& rect) const;
            const Point screenToLngLat(const Point& screenPoint);           // Temporary test, should be removed
            const Point mapToLngLat(const Point& mapPoint);                 // Temporary test, should be removed
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

            void addMapEventHandler(MapEventHandler* mapEventHandler) { m_mapEventHandler = mapEventHandler; }
            void removeMapEventHandler() { m_mapEventHandler = nullptr; }

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
            bool isSelected(FeaturePtr feature);
            void hover(const Id& id);
            void hover(FeaturePtr feature);
            void hover(const std::vector<Id>& ids);
            void hover(const std::vector<FeaturePtr>& features);
            std::vector<Id> hovered() { return m_hoveredFeatures; };
            bool isHovered(const Id& id);
            bool isHovered(FeaturePtr feature);

            void resize();
            Drawable& drawable();

            // Update flags
            bool centerChanged() { return m_centerChanged; }
            bool scaleChanged() { return m_scaleChanged; }
            bool rotationrChanged() { return m_rotationChanged; }

            void startInitialAnimation();
        private:
            void beforeRender();
            void afterRender();
            
            void resetUpdateFlags();

            void drawInfo();
            
            cimg_library::CImgDisplay& m_disp;

            Raster m_backgroundRaster;
            Drawable m_drawable;
            
            Point m_center;
            double m_scale;
            double m_rotation;

            MapConstraints m_constraints;

            bool m_updateRequired;
            bool m_updateEnabled;
            bool m_quickUpdateEnabled;

            // Update flags
            bool m_centerChanged;
            bool m_scaleChanged;
            bool m_rotationChanged;

            MapEventHandler* m_mapEventHandler;

            AnimationPtr    m_animation;
            int m_animationStartTimeStamp;

            std::vector<Layer*> m_layers;
            std::vector<PresentationObject> m_presentationObjects;
            std::vector<Id>                 m_selectedFeatures;
            std::vector<Id>                 m_hoveredFeatures;
    };

    class MapEventHandler
    {
        public:
            virtual void OnAreaChanged(Map& map) = 0;
            virtual void OnUpdating(Map& map) = 0;
            virtual void OnCustomDraw(Map& map) = 0;
            virtual void OnUpdated(Map& map) = 0;
    };
}

#endif /* BLUEMARBLE_MAP */
