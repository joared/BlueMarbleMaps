#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/MapConstraints.h"
#include "BlueMarbleMaps/Utility/Utils.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include <cmath>
#include <iostream>
#include <vector>
#include <set>

using namespace BlueMarble;

constexpr double xPixLen = 0.02222222222222;
constexpr double yPixLen = -0.02222222222222;
constexpr double xTopLeft = -179.98888888888889;
constexpr double yTopLeft = 89.98888888888889;

// constexpr double xPixLen = 0.03333333333333;
// constexpr double yPixLen = -0.03333333333333;
// constexpr double xTopLeft = -179.98333333333333;
// constexpr double yTopLeft = 89.98333333333333;

Map::Map()
    : m_center(lngLatToMap(Point(30, 36)))
    , m_scale(1.0)
    , m_rotation(0.0)
    , m_constraints(5000.0, 0.0001, Rectangle(0, 0, 100000, 100000))
    , m_crs(Crs::wgs84LngLat())
    , m_updateRequired(true)
    , m_updateEnabled(true)
    , m_animation(nullptr)
    , m_animationStartTimeStamp(0)
    , m_updateAttributes()
    , m_centerChanged(false)
    , m_scaleChanged(false)
    , m_rotationChanged(false)
    , m_presentationObjects()
    , m_selectedFeatures()
    , m_hoveredFeatures()
    , m_commmand(nullptr)
    , m_showDebugInfo(true)
    , m_isUpdating(false)
    , m_renderingEnabled(true)
{   
    m_drawable = std::make_shared<SoftwareBitmapDrawable>(500, 500, 4);
    m_presentationObjects.reserve(1000000); // Reserve a good amount for efficiency
    resetUpdateFlags();
    m_constraints.bounds().scale(3.0);

    updateUpdateAttributes(getTimeStampMs());
}

bool Map::update(bool forceUpdate)
{   
    if (!forceUpdate && m_mapControl)
    {
        // Let MapControl schedule update
        // Note: this is allowed to be called within an update
        m_mapControl->updateView();
        return true;
    }

    assert(!m_isUpdating);
    m_isUpdating = true;
    // TODO: should be handled as int64_t
    int timeStampMs = getTimeStampMs();
    if (!m_updateEnabled || (!forceUpdate && !m_animation && !m_updateRequired))
    {
        std::cout << "Map::update() No update required!\n";
        return false;
    }
    updateUpdateAttributes(timeStampMs); // Set update attributes that contains useful information about the update

    // FIXME: should animations be before onUpdating?
    if(m_animation && m_animation->update(timeStampMs - m_animationStartTimeStamp))
    {
        // Animation finished
        stopAnimation();
    }

    events.onUpdating.notify(*this);
    
    // Constrain map pose
    m_constraints.constrainMap(*this);

    // Rendering
    beforeRender();
    renderLayers(); // Let layers do their work

    // Each onCustomDraw notification should have the transform set to "screen".
    // Since handlers are allowed to modify the transform, we need to make sure to reset it
    // each time a handler is called.
    auto preNotifyAction = [this]() 
    { 
        m_drawable->setTransform(Transform::screenTransform(m_drawable->width(), m_drawable->height())); 
    };
    events.onCustomDraw.notify(*this, preNotifyAction);

    if (m_showDebugInfo)
    {
        drawDebugInfo(getTimeStampMs() - timeStampMs);
    }

    afterRender();
    
    events.onUpdated.notify(*this);
    
    m_updateRequired = m_updateAttributes.get<bool>(UpdateAttributeKeys::UpdateRequired); // Someone in the operator chain needs more updates (e.g. Visualization evaluations)
    //m_updateRequired = m_updateRequired || m_animation != nullptr;
    resetUpdateFlags();

    m_isUpdating = false;

    bool updateRequired = m_updateRequired || m_animation != nullptr;
    if (!updateRequired)
    {
        events.onIdle.notify(*this);
    }

    return updateRequired;
}


void Map::renderLayers()
{
    m_presentationObjects.clear(); // Clear presentation objects, layers will add new
    auto updateArea = area();
    updateArea.scale(.5);

    FeatureQuery featureQuery;
    featureQuery.area(updateArea);
    featureQuery.scale(scale());
    featureQuery.quickUpdate(quickUpdateEnabled());
    featureQuery.updateAttributes(&updateAttributes());

    for (auto l : m_layers)
    {
        // TODO add "ViewInfo" as parameter to Layer::update()?
        l->update(shared_from_this(), getCrs(), featureQuery);
        m_drawable->drawText(0,0,"",Color(), 2); // Faking flush
    }

    // Debug draw update area
    auto line = std::make_shared<LineGeometry>(updateArea);
    Pen p;
    p.setColor(Color::red());
    p.setThickness(5.0);
    //m_drawable->setTransform(Transform::screenTransform(m_drawable->width(), m_drawable->height()));
    m_drawable->drawLine(line, p);
    m_drawable->drawText(0,0,"",Color(), 2); // Faking flush
}

void Map::center(const Point &center)
{
    m_updateRequired = true;
    m_centerChanged = true;
    m_center = center;
}

void BlueMarble::Map::scale(double scale)
{
    m_updateRequired = true;
    m_scaleChanged = true;
    m_scale = scale;
}

double BlueMarble::Map::scale() const
{
    return m_scale;
}

double BlueMarble::Map::invertedScale() const
{
    return 1.0 / scale();
}

void BlueMarble::Map::invertedScale(double invScale)
{
    scale(1.0 / invScale);
}

double BlueMarble::Map::rotation() const
{
    return m_rotation;
}

void BlueMarble::Map::rotation(double rotation)
{
    m_updateRequired = true;
    m_rotationChanged = true;
    m_rotation = rotation;
}

double BlueMarble::Map::width() const
{
    return m_drawable->width() / scale(); // TODO: should we use raster width instead?
}

double BlueMarble::Map::height() const
{
    return m_drawable->height() / scale(); // TODO: should we use raster height instead?
}

Rectangle BlueMarble::Map::area() const
{
    return Rectangle
    (
        center().x() - (width()/*-1*/)*0.5,     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().y() - (height()/*-1*/)*0.5,    // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().x() + (width()/*-1*/)*0.5,     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
        center().y() + (height()/*-1*/)*0.5     // TODO: minus one when pixel coordinates? If so, geCrop needs to be adjusted in render()
    );
}

void Map::panBy(const Point& deltaScreen, bool animate)
{
    //center(m_center + Point(deltaScreen.x(), deltaScreen.y())*(1/m_scale));
    // TODO: add this back when fixed, or?
    auto centerScreen = screenCenter();
    auto newScreenCenter = centerScreen + deltaScreen;
    auto to = screenToMap(newScreenCenter.x(), newScreenCenter.y());

    if (animate)
    {
        auto animation = Animation::Create(*this, center(), to, 500, false);
        startAnimation(animation);
    }
    else
    {
        center(to);
    }
}

void Map::panTo(const Point& mapPoint, bool animate)
{
    if (animate)
    {
        auto animation = Animation::Create(*this, center(), mapPoint, scale(), scale(), 1000, false);
        startAnimation(animation);
    }
    else
    {
        center(mapPoint);
    }
}

void Map::zoomTo(const Point& newCenter, double newScale, bool animate)
{
    if (animate)
    {
        auto animation = Animation::Create(*this, center(), newCenter, scale(), newScale, 1000, false);
        startAnimation(animation);
    }
    else
    {
        center(newCenter);
        scale(newScale);
    }
}

void Map::zoomOn(const Point& mapPoint, double zoomFactor, bool animate)
{
    // std::cout << "zoomOn\n";
    if (animate && m_animation)
    {
        zoomFactor = m_animation->toScale()/scale() * zoomFactor;
    }

    double newScale = m_constraints.constrainValue(scale()*zoomFactor, 
                                                   m_constraints.minScale(), 
                                                   m_constraints.maxScale());
    zoomFactor = newScale / scale();

    auto delta = Point((mapPoint.x() - center().x()), 
                       (mapPoint.y() - center().y()));
    
    auto newCenter = mapPoint - delta*(1.0/zoomFactor);

    zoomTo(newCenter, newScale, animate);
    // if (animate)
    // {
    //     auto animation = Animation::Create(*this, center(), newCenter, scale(), newScale, 300, false);
    //     startAnimation(animation);
    // }
    // else
    // {
    //     center(newCenter);
    //     scale(newScale);
    // }
}

void BlueMarble::Map::zoomToArea(const Rectangle& bounds, bool animate)
{
    std::cout << "zoomToArea\n";
    auto to = bounds.center();

    double toScale = scale() * width() / bounds.width();
    if (width() / height() > bounds.width() / bounds.height())
        toScale = scale() * height() / bounds.height();

    toScale = m_constraints.constrainValue(toScale, 
                                           m_constraints.minScale(), 
                                           m_constraints.maxScale());

    //zoomTo(to, toScale, animate);
    if (animate)
    {
        auto animation = Animation::Create(*this, center(), to, scale(), toScale, 1000, false);
        startAnimation(animation);
    }
    else
    {
        center(to);
        scale(toScale);
    }
}

void BlueMarble::Map::zoomToMinArea(const Rectangle &bounds, bool animate)
{
    auto to = bounds.center();

    double toScale = scale() * width() / bounds.width();
    if (width() / height() < bounds.width() / bounds.height())
        toScale = scale() * height() / bounds.height();

    toScale = m_constraints.constrainValue(toScale, 
                                           m_constraints.minScale(), 
                                           m_constraints.maxScale());

    if (animate)
    {
        auto animation = Animation::Create(*this, center(), to, scale(), toScale, 1000, false);
        startAnimation(animation);
    }
    else
    {
        center(to);
        scale(toScale);
    }
}

Point BlueMarble::Map::screenToMap(const Point& screenPos) const
{
    return screenToMap(screenPos.x(), screenPos.y());
}

Point BlueMarble::Map::screenToMap(double x, double y) const
{
    // NOTE: this methods treats screen coordinates as "pixel indexes", not the geometrical point
    auto sCenter = screenCenter();
    double mapX = (x - sCenter.x()) / m_scale + m_center.x();// + m_img.width() / 2.0;
    double mapY = -(y - sCenter.y()) / m_scale + m_center.y();// + m_img.height() / 2.0;

    return Utils::rotatePointDegrees(Point(mapX, mapY), m_rotation, m_center);
}

Point Map::mapToScreen(const Point& point) const
{
    // NOTE: this methods treats screen coordinates as "pixel indexes", not the geometrical point
    auto screenC = screenCenter();
    //auto screenC = Point(m_drawable->width()*0.5, m_drawable->height()*0.5);

    auto delta = Utils::rotatePointDegrees(point, -m_rotation, m_center) - m_center;
    double x = delta.x()*m_scale + screenC.x();
    double y = -delta.y()*m_scale + screenC.y();

    return Point(x, y);
}

std::vector<Point> BlueMarble::Map::screenToMap(const std::vector<Point> &points) const
{
    std::vector<Point> mapPoints;
    for (auto& p : points)
    {
        mapPoints.push_back(screenToMap(p));
    }

    return mapPoints;
}

std::vector<Point> BlueMarble::Map::mapToScreen(const std::vector<Point> &points) const
{
    std::vector<Point> screenPoints;
    for (auto& p : points)
    {
        screenPoints.push_back(mapToScreen(p));
    }
    return screenPoints;
}

std::vector<Point> BlueMarble::Map::lngLatToMap(const std::vector<Point> &points) const
{
    std::vector<Point> mapPoints;
    for (auto& p : points)
    {
        mapPoints.push_back(lngLatToMap(p));
    }

    return mapPoints;
}

Rectangle BlueMarble::Map::screenToMap(const Rectangle &rect) const
{
    auto topLeft = screenToMap(Point(rect.xMin(), rect.yMin()));
    auto bottomRight = screenToMap(Point(rect.xMax(), rect.yMax()));
    
    return Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

Rectangle BlueMarble::Map::mapToScreen(const Rectangle &rect) const
{
    auto topLeft = mapToScreen(Point(rect.xMin(), rect.yMin()));
    auto bottomRight = mapToScreen(Point(rect.xMax(), rect.yMax()));
    
    return Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

Point Map::screenCenter() const
{
    return Point((m_drawable->width()-1)*0.5, (m_drawable->height()-1)*0.5);
}

void Map::startAnimation(AnimationPtr animation)
{
    std::cout << "Started animation\n";
    if (m_animation)
        stopAnimation();
    m_animation = animation;
    m_animationStartTimeStamp = getTimeStampMs();
    quickUpdateEnabled(true); // For now, consider animations as quick updates
}

void Map::stopAnimation()
{
    if (m_animation)
    {
        std::cout << "Stoppped animation\n";
    }
    
    m_animation = nullptr;
    quickUpdateEnabled(false);
}

void Map::addLayer(const LayerPtr& layer)
{
    assert(layer != nullptr);
    m_layers.push_back(layer);
}

std::vector<LayerPtr>& Map::layers()
{
    return m_layers;
}

FeaturePtr Map::getFeature(const Id &id)
{
    for (auto l : m_layers)
    {
        if (auto f = l->getFeature(id))
            return f;
    }
    
    return nullptr;
}

// void Map::getFeatures(const Attributes& attributes, std::vector<FeaturePtr>& features)
// {
//     for (auto l : m_layers)
//     {
//         l->onGetFeaturesRequest(attributes, features);    
//     }
// }

std::vector<FeaturePtr> Map::featuresAt(int X, int Y, double pointerRadius)
{
    std::vector<FeaturePtr> hitFeatures;
    for (auto& po : hitTest(X, Y, pointerRadius))
    {
        // Only push unique features. NOTE: we don't use a set since it does not retain the order
        bool alreadyExists = false;
        for(auto f : hitFeatures)
        {
            if (f == po.sourceFeature())
                alreadyExists = true;
        }
        if(!alreadyExists)
            hitFeatures.push_back(po.sourceFeature());
    }

    return hitFeatures;
}

void BlueMarble::Map::featuresInside(const Rectangle& bounds, FeatureCollection& hitFeatures)
{
    assert(hitFeatures.size() == 0);

    for (auto& po : hitTest(bounds))
    {
        // Only push unique features. NOTE: we don't use a set since it does not retain the order
        bool alreadyExists = false;
        for(auto f : hitFeatures)
        {
            if (f == po.sourceFeature())
                alreadyExists = true;
        }
        if(!alreadyExists)
            hitFeatures.add(po.sourceFeature());
    }
}

const std::vector<PresentationObject>& BlueMarble::Map::hitTest(int x, int y, double pointerRadius)
{
    auto area = Rectangle({(double)x,(double)y}, pointerRadius, pointerRadius);
    area = screenToMap(area);

    return hitTest(area);
}

const std::vector<PresentationObject>& BlueMarble::Map::hitTest(const Rectangle& bounds)
{
    FeatureQuery featureQuery;
    featureQuery.area(bounds);
    featureQuery.scale(scale());
    featureQuery.updateAttributes(&updateAttributes());

    m_presentationObjects.clear();
    for (const auto& l : m_layers)
    {
        auto features = l->getFeatures(m_crs, featureQuery, true);

        while (features->moveNext())
        {
            const auto& f = features->current();

            for (const auto& vis : l->visualizers())
            {
                vis->hitTest(f, drawable(), bounds, m_presentationObjects);
            }

            if (isSelected(f->id()))
            {
                for (const auto& vis : l->selectionVisualizers())
                {
                    vis->hitTest(f, drawable(), bounds, m_presentationObjects);
                }
            }
            else if (isHovered(f->id()))
            {
                for (const auto& vis : l->hoverVisualizers())
                {
                    vis->hitTest(f, drawable(), bounds, m_presentationObjects);
                }
            }
        }
    }

    // Iterate in reverse order such that the first rendered presentation objects are first
    // for (auto it=m_presentationObjects.end()-1; it!=m_presentationObjects.begin()-1; it--)
    // {
    //     auto& p = *it;
    //     if (p.hitTest(bounds))
    //     {
    //         hitObjects.push_back(p);
    //     }
    // }

    return m_presentationObjects;
}

void Map::select(FeaturePtr feature, SelectMode mode)
{
    if(!feature)
        return;

    switch (mode)
    {
    case SelectMode::Replace:
        m_selectedFeatures.clear();
        break;
    
    case SelectMode::Add:
        break;

    default:
        std::cout << "Map::select() Unhandled select mode: " << (int)mode << "\n";
        break;
    }

    if (!isSelected(feature))
    {
        m_selectedFeatures.push_back(feature->id());
        
        // Testing restartVisualizationAnimation
        if (auto dataSet = DataSet::getDataSetById(feature->id().dataSetId()))
        {
            dataSet->restartVisualizationAnimation(feature, getTimeStampMs());
        }

        // std::cout << "Selected feature, Id: " << "Id(" << feature->id().dataSetId() << ", " << feature->id().featureId() << ")\n";
        std::cout << feature->prettyString();
    }
}

void Map::select(const PresentationObject& presentationObject)
{
    m_selectedPresentationObjects.push_back(presentationObject);
}

const std::vector<PresentationObject>& Map::selectedPresentationObjects()
{
    return m_selectedPresentationObjects;
}

void Map::deSelect(FeaturePtr feature)
{
    for (auto it = m_selectedFeatures.begin(); it!= m_selectedFeatures.end(); it++)
    {
        if(*it == feature->id())
        {
            m_selectedFeatures.erase(it);
            return;
        }
    }
}

void Map::deSelectAll()
{
    m_selectedFeatures.clear();
    m_selectedPresentationObjects.clear();
}

bool Map::isSelected(const Id& id)
{
    for (auto id2 : m_selectedFeatures)
    {
        if (id == id2)
            return true;
    }

    return false;
}

bool Map::isSelected(FeaturePtr feature)
{
    return isSelected(feature->id());
}

void Map::hover(const Id& id)
{
    if (isHovered(id))
        return;
    m_hoveredFeatures.clear();
    if (id != Id(0,0))
        m_hoveredFeatures.push_back(id);
}

void Map::hover(FeaturePtr feature)
{
    Id id(0,0);
    if (feature)
    {
        id = feature->id();
    }

    // Testing visualization animation
    if (!isHovered(id) && !isSelected(id))
    {
        if (auto dataSet = DataSet::getDataSetById(id.dataSetId()))
        {
            dataSet->restartVisualizationAnimation(feature, getTimeStampMs());
            BMM_DEBUG() << "Hover restart (id: " << id.toString() << ")\n";
        }
    }
    hover(id);
}

void Map::hover(const std::vector<Id>& ids)
{
    m_hoveredFeatures = ids;
}

void Map::hover(const std::vector<FeaturePtr>& features)
{
    m_hoveredFeatures.clear();
    for (auto f : features)
    {
        m_hoveredFeatures.push_back(f->id());
    }
}

bool BlueMarble::Map::isHovered(const Id& id)
{
    for (auto id2 : m_hoveredFeatures)
    {
        if(id == id2)
            return true;
    }
    return false;
}

bool BlueMarble::Map::isHovered(FeaturePtr feature)
{
    if (!feature)
        return m_hoveredFeatures.empty();
    return isHovered(feature->id());
}

BlueMarble::DrawablePtr Map::drawable()
{
    // TODO: should be own m_drawable
    return m_drawable;
}

void BlueMarble::Map::drawable(const DrawablePtr &drawable)
{
    m_drawable = drawable;
}

void BlueMarble::Map::renderingEnabled(bool enabled)
{
    m_renderingEnabled = enabled;
    for (const auto& l : m_layers)
    {
        l->renderingEnabled(enabled);
    }
}

void Map::updateUpdateAttributes(int64_t timeStampMs)
{
    m_updateAttributes.set(UpdateAttributeKeys::UpdateTimeMs, (int)timeStampMs);
    m_updateAttributes.set(UpdateAttributeKeys::UpdateViewScale, scale());
    m_updateAttributes.set(UpdateAttributeKeys::QuickUpdate, false);
    m_updateAttributes.set(UpdateAttributeKeys::SelectionUpdate, false);
    m_updateAttributes.set(UpdateAttributeKeys::HoverUpdate, false);
    if (!m_selectedFeatures.empty())
        m_updateAttributes.set(UpdateAttributeKeys::SelectionUpdate, true);
    if (!m_hoveredFeatures.empty())
    {
        m_updateAttributes.set(UpdateAttributeKeys::HoverUpdate, true);
    }

    // For others in the operator chain to set (or elsewhere).
    // These are reset in the beginning of each update
    m_updateAttributes.set(UpdateAttributeKeys::UpdateRequired, false);
    
}

void Map::beforeRender()
{
    m_drawable->clearBuffer();
    auto sc = screenCenter();
    sc = Point(sc.x(), sc.y());
    
    auto center = Point(
        m_center.x(),// - (float)width()  / (2.0f * (float)m_scale),
        m_center.y()// - (float)height() / (2.0f * (float)m_scale)
    );
    m_drawable->setTransform(Transform(center, m_scale, m_scale, m_rotation));
}

void Map::afterRender()
{
    m_drawable->swapBuffers();
}

void Map::resetUpdateFlags()
{
    m_centerChanged = false;
    m_scaleChanged = false;
    m_rotationChanged = false;
}

void Map::startInitialAnimation()
{
    auto animation = Animation::Create(*this, 
                                       center(),
                                       center(),
                                       scale()*10.0,
                                       scale(),
                                       1000,
                                       false);
    startAnimation(animation);                                
}

void BlueMarble::Map::doCommand(const std::function<void()>& action)
{
    delete m_commmand;
    m_commmand = new MapCommand(*this, action);
    m_commmand->execute();
}

bool BlueMarble::Map::undoCommand()
{
    if(m_commmand)
    {
        m_commmand->revert();
        delete m_commmand;
        m_commmand = nullptr;
        return true;
    }
    return false;
}

void Map::drawDebugInfo(int elapsedMs)
{
    ScreenPos mousePos;
    if (m_mapControl)
        m_mapControl->getMousePos(mousePos);
    auto mouseMapPos = screenToMap(mousePos.x, mousePos.y);
    auto mouseLngLat = mapToLngLat(mouseMapPos);
    auto centerLngLat = mapToLngLat(center());
    auto screenPos = mapToScreen(mouseMapPos).round();
    auto screenError = Point(mousePos.x-(int)screenPos.x(), mousePos.y-(int)screenPos.y());
    std::string info = "------ Debug -------\n";
    info += "Center: " + std::to_string(m_center.x()) + ", " + std::to_string(m_center.y());
    info += "\nCenter LngLat: " + std::to_string(centerLngLat.x()) + ", " + std::to_string(centerLngLat.y());
    info += "\nScale: " + std::to_string(m_scale);
    info += "\nScale inv: " + std::to_string(invertedScale());
    
    if (std::abs(screenError.x()) > 0 || std::abs(screenError.y()) > 0)
    {
        // Only display this when error occurs
        info += "\nMouse: " + std::to_string(mousePos.x) + ", " + std::to_string(mousePos.y);
        info += "\nMouseToMap: " + std::to_string(mouseMapPos.x()) + ", " + std::to_string(mouseMapPos.y());
        info += "\nMouseToMapToMouse: " + std::to_string((int)screenPos.x()) + ", " + std::to_string((int)screenPos.y());
        info += "\nScreen error: " + std::to_string(mousePos.x-(int)screenPos.x()) + ", " + std::to_string(mousePos.y-(int)screenPos.y());
        info += "\nMouseLngLat: " + std::to_string(mouseLngLat.x()) + ", " + std::to_string(mouseLngLat.y());
    }

    info += "\nUpdate time: " + std::to_string(elapsedMs);
    info += "\nPresentationObjects: " + std::to_string(m_presentationObjects.size());
    
    info += "\n";
    // auto presentationObjects = hitTest(mousePos.x, mousePos.y, 10.0);
    // for (auto& p : presentationObjects)
    // {
    //     info += "\t";
    //     info += "Geometry: " + typeToString(p.feature()->geometryType());
    //     info += ", Source: " + typeToString(p.sourceFeature()->geometryType());
    //     info += ", Node: " + std::to_string(p.nodeIndex());
    //     info += "\n";
    // }
    

    int fontSize = 16;
    m_drawable->drawText(0, 0, info.c_str(), Color(0, 0, 0), fontSize);

    BMM_DEBUG() << info << "\n\n";
}

const Point Map::screenToLngLat(const Point& screenPoint)
{
    return mapToLngLat(screenToMap(screenPoint));
}

const Point Map::mapToLngLat(const Point& mapPoint, bool normalize)
{
    // This is temporary, remove.
    // Using georeferencing parameters from NE1_LR_LC_SR_W.tfw

    // Reference: https://help.allplan.com/Allplan/2016-1/1033/Allplan/51691.htm
    // Line 1: length of a pixel in the x direction (horizontal)
    // Line 2: angle of rotation (is usually 0 or ignored)
    // Line 3: angle of rotation (is usually 0 or ignored)
    // Line 4: negative length of a pixel in the y direction (vertical)
    // Line 5: x coordinate at the center of the pixel in the top left corner of the image
    // Line 6: y coordinate at the center of the pixel in the top left corner of the image
    

    // The origin is defined at the center of the top left pixel.
    // Map coordinates (image coordinates) are currently center in the top left corner
    // of the top left pixel, so we subtract half a pixel to account for this.
    // auto imagePoint = Point(mapPoint.x()-0.5, mapPoint.y()-0.5);

    // double lng = xTopLeft + xPixLen * imagePoint.x();
    // double lat = yTopLeft + yPixLen * imagePoint.y();
    
    // if (normalize)
    // {
    //     lng = Utils::normalizeLongitude(lng);
    //     lat = Utils::normalizeLatitude(lat); // FIXME: y will change sign
    // }

    // return Point(lng, lat);
    return mapPoint;
}

const Point BlueMarble::Map::lngLatToMap(const Point& lngLat) const
{
    // double x = (lngLat.x() - xTopLeft) / xPixLen + 0.5;
    // double y = (lngLat.y() - yTopLeft) / yPixLen + 0.5;

    // return Point(x, y);
    return lngLat;
}

const Point BlueMarble::Map::lngLatToScreen(const Point& lngLat)
{
    return mapToScreen(lngLatToMap(lngLat));
}

std::vector<Point> BlueMarble::Map::lngLatToScreen(const std::vector<Point> &points)
{   
    std::vector<Point> newPoints;
    for (auto& p : points)
    {
        newPoints.push_back(lngLatToScreen(p));
    }

    return newPoints;
}

Rectangle BlueMarble::Map::lngLatToMap(const Rectangle &rect)
{
    auto topLeft = lngLatToMap(Point(rect.xMin(), rect.yMin()));
    auto bottomRight = lngLatToMap(Point(rect.xMax(), rect.yMax()));
    
    return Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

Rectangle BlueMarble::Map::mapToLngLat(const Rectangle &rect)
{
    auto topLeft = mapToLngLat(Point(rect.xMin(), rect.yMin()), false);     // FIXME: not sure if not normalizing is always correct
    auto bottomRight = mapToLngLat(Point(rect.xMax(), rect.yMax()), false); // FIXME: not sure if not normalizing is always correct
    
    return Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

Rectangle BlueMarble::Map::lngLatToScreen(const Rectangle &rect)
{
    auto topLeft = lngLatToScreen(Point(rect.xMin(), rect.yMin()));
    auto bottomRight = lngLatToScreen(Point(rect.xMax(), rect.yMax()));
    
    return Rectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

const Point &Map::center() const
{
    return m_center;
}