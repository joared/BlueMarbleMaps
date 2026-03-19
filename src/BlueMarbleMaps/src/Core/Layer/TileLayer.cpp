#include "BlueMarbleMaps/Core/Layer/TileLayer.h"
#include "BlueMarbleMaps/Core/Layer/StandardLayer.h"
#include "BlueMarbleMaps/Core/Map.h"


using namespace BlueMarble;

#define TILELAYER_TILE_SIZE 512
#define TILELAYER_NUM_WORKERS std::thread::hardware_concurrency()
#define TILELAYER_QUEUE_SIZE 4
#define TILELAYER_QUEUE_POLICY System::ThreadPool::QueuePolicy::ReplaceOldestWhenFull

TileManager::TileManager(const Rectangle& fullExtent)
    : m_tilingScheme(fullExtent)
    , m_tileCache()
{
}

Rectangle TileManager::tileBounds(int x, int y, int zoom) const
{
    return m_tilingScheme.tileBounds(x, y, zoom);
}

std::vector<Tile> TileManager::getTilesForArea(const Rectangle& area, int zoom) const
{
    return m_tilingScheme.getTilesForArea(area, zoom);
}

// Fetches tile data for a given tile
std::vector<Tile> TileManager::getCachedTiles(const Rectangle& area, int zoom)
{
    // Placeholder implementation, in a real implementation this would check the cache and return any tiles that are already loaded
    if (m_tileCache.empty())
    {
        return {};
    }
    std::vector<Tile> tiles;
    auto areaTiles = m_tilingScheme.getTilesForArea(area, zoom);
    
    for (const Tile& t : areaTiles)
    {
        auto it = m_tileCache.find(t.id());
        if (it != m_tileCache.end())
        {
            if (it->second.features) // Only return tiles that have their features loaded
                tiles.push_back(it->second);
        }
    }

    return tiles;
}

const Tile& TileManager::getCachedTile(const Tile& tile) const
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::getCachedTile Invalid tile!!!" << tile.toString() << "\n";
    // }
    return m_tileCache.at(tile.id());
}

bool TileManager::hasTile(const Tile& tile) const
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::hasTile Invalid tile!!!" << tile.toString() << "\n";
    // }
    auto it = m_tileCache.find(tile.id());
    return it != m_tileCache.end();
}

bool TileManager::hasLoadedTile(const Tile& tile) const
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::hasLoadedTile Invalid tile!!!" << tile.toString() << "\n";
    // }

    auto it = m_tileCache.find(tile.id());
    if (it == m_tileCache.end())
    {
        return false;
    }
    const auto& [id, t] = *it;
    return t.isLoaded();
}

void TileManager::setTile(Tile&& tile)
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::setTile Invalid tile!!!\n";
    // }
    // TODO: sometimes features can be loaded at the wrong tile. Reproduce by using small tile size and many background threads
    if (hasTile(tile))
    {
        if (m_tileCache.at(tile.id()).features != nullptr)
        {
            throw std::runtime_error("TileManager::setTile() tile already exists!");
        }
        m_tileCache[tile.id()] = std::move(tile);
    }
    else
    {
        m_tileCache.emplace(tile.id(), std::move(tile));
    }
}

void TileManager::markTileDirty(const Tile &tile)
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::markTileDirty Invalid tile!!!" << tile.toString() << "\n";
    // }
    m_tileCache.at(tile.id()).features = nullptr;
}

void TileManager::removeTile(const Tile &tile)
{
    // if (!tile.isValid())
    // {
    //     BMM_DEBUG() << "TileManager::removeTile Invalid tile!!!" << tile.toString() << "\n";
    // }
    m_tileCache.erase(tile.id());
}

TileLayer::TileLayer()
    : LayerSet()
    , m_threadPool()
    , m_tileManager(nullptr)
    , m_readAsync(true)
    , m_tileSize(TILELAYER_TILE_SIZE)
{
    if (m_readAsync)
    {
        // TODO: make these parameters configurable
        m_threadPool.start(TILELAYER_NUM_WORKERS, TILELAYER_QUEUE_SIZE, TILELAYER_QUEUE_POLICY);
        
        // TODO: add pruning of cache to prevent memory explosion
        // m_threadPool.enqueue([this]{
        //     cleanCache();
        // });
    }
}

void TileLayer::asyncRead(bool async)
{
    // TODO: must add parameters for thread pool
    if (m_readAsync && !async)
    {
        m_threadPool.stop();
    }
    else if (!m_readAsync && async)
    {
        m_threadPool.start(TILELAYER_NUM_WORKERS, TILELAYER_QUEUE_SIZE, TILELAYER_QUEUE_POLICY);
    }

    m_readAsync = async;
}

FeatureEnumeratorPtr TileLayer::prepare(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    if (!isActiveForQuery(featureQuery))
    {
        return std::make_shared<FeatureEnumerator>();
    }

    if (!m_tileManager)
    {
        m_tileManager = std::make_unique<TileManager>(crs->bounds());
        verifyValidSubLayers();
    }

    if (!m_readAsync)
    {
        return LayerSet::prepare(crs, featureQuery);
    }
    
    int tileSize = m_tileSize;
    double zoom0Resolution = crs->bounds().width() / (double)tileSize;
    double unitsPerPixel = Drawable::pixelSize() / crs->globalMetersPerUnit() / featureQuery.scale();
    int zoom = static_cast<int>(std::floor(std::log2(zoom0Resolution/unitsPerPixel)));
    zoom = std::clamp(zoom, 0, 20); // TODO: make these parameters configurable
    unitsPerPixel = zoom0Resolution / std::pow(2.0, zoom); // clamp unitsperpix
    
    double clampedScale = Drawable::pixelSize() / crs->globalMetersPerUnit() / (zoom0Resolution / std::pow(2.0, zoom));

    // BMM_DEBUG() << "TileLayer::prepare() Units per pixel: " << unitsPerPixel << ", Zoom level: " << zoom << "\n";

    std::vector<Tile> tiles;
    {
        std::lock_guard lock(m_mutex);
        tiles = m_tileManager->getTilesForArea(featureQuery.area(), zoom);
    }

    FeatureEnumeratorPtr enumerator = std::make_shared<FeatureEnumerator>();
    for (int i(0); i<layers().size(); ++i)
    {
        enumerator->addEnumerator(std::make_shared<FeatureEnumerator>());   
    }

    std::unordered_map<Id, bool, Id::IdHash> featuresAdded; // Used to avoid adding the same feature multiple times if it appears in multiple tiles
    
    std::lock_guard lock(m_mutex);
    for (Tile& tile : tiles)
    {
        if (!m_tileManager->hasTile(tile))
        {
            // If the tile is not in the cache, we need to load it asynchronously
            auto tileQuery = featureQuery;
            tileQuery.area(m_tileManager->tileBounds(tile.x, tile.y, tile.zoom));
            tileQuery.scale(clampedScale);

            // TODO: Needs debugging together with ImageDataSet::onGetFeatures()
            // Its needed when crs differ, but seems slower if theyre not.
            // For datasets that dont have the same crs as requested, its unnecessary to do "clone"
            // when reqprojecting since we get a copy anyway.
            tileQuery.rasterGeometryMode(FeatureQuery::RasterGeometryMode::Clipped);
            tileQuery.resolution(unitsPerPixel);

            scheduleTileLoad(tile, crs, tileQuery);
        }

        if (!m_tileManager->hasLoadedTile(tile))
        {
            // Almost working
            Tile parent = tile;
            while (m_tileManager->parentOf(parent, parent))
            {
                if (m_tileManager->hasLoadedTile(parent))
                {
                    // BMM_DEBUG() << "Found PARENT of " << tile.toString() << ": " << parent.toString() << "\n";
                    tile = parent;
                    break;
                }
            }
        }

        if (m_tileManager->hasLoadedTile(tile))
        {
            // If the tile is in the cache, we can add its features to the enumerator
            
            auto cachedTile = m_tileManager->getCachedTile(tile);
            if (cachedTile.features)
            {   
                // BMM_DEBUG() << "Preparing tile: " << tile.toString() << "\n";
                cachedTile.features->reset();

                // TODO optimize
                int n = cachedTile.features->subEnumerators().size();
                for (int i(0); i<n; ++i)
                {
                    while (cachedTile.features->subEnumerators()[i]->moveNext())
                    {
                        const auto& f = cachedTile.features->subEnumerators()[i]->current();
                        if (featuresAdded.find(f->id()) == featuresAdded.end())
                        {
                            enumerator->subEnumerators()[i]->add(f);
                            
                            // featuresAdded[f->id()] = true;
                            if (f->geometryType() != GeometryType::Raster)
                            {
                                featuresAdded[f->id()] = true;
                            }
                        }
                    }
                    cachedTile.features->subEnumerators()[i]->reset();
                }

                cachedTile.features->reset();
            }
        }
    }

    return enumerator;
}

void TileLayer::update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery)
{
    // // For now, delegate to LayerSet's update with the prepared features
    LayerSet::update(map, features, featureQuery);

    bool drawDebugTiles = false; // TODO: make this configurable
    if (drawDebugTiles)
    {
        drawTiles(map, featureQuery);
    }
}

void TileLayer::flushCache()
{
    {
        std::lock_guard lock(m_mutex);
        m_tileManager = nullptr;
    }
    m_threadPool.stop(true);
    
    LayerSet::flushCache();

    m_threadPool.start(TILELAYER_NUM_WORKERS, TILELAYER_QUEUE_SIZE, TILELAYER_QUEUE_POLICY);
}

void TileLayer::verifyValidSubLayers()
{
    // Sublayers should not have async reading under a tile layer.
    // For now, we throw exception
    std::function<void(const LayerPtr& layer)> checkLayer;

    checkLayer = [&checkLayer](const auto& layer)
    {
        if (auto standard = std::dynamic_pointer_cast<StandardLayer>(layer))
        {
            if (standard->asyncRead())
            {
                throw std::runtime_error("Found StandardLayer under TilaLayer with async read. It's not allowed!");
            }
        }
        else if (auto layerSet = std::dynamic_pointer_cast<LayerSet>(layer))
        {
            if (auto tileLayer = std::dynamic_pointer_cast<TileLayer>(layer))
            {
                if (tileLayer->asyncRead())
                {
                    throw std::runtime_error("Found TileLayer under TileLayer with async read. It's not!");
                }
            }
            for (const auto& l : layerSet->layers())
            {
                checkLayer(l);
            }
        }
        // TODO: check TileLayer as well
    };

    for (const auto& l : layers())
    {
        checkLayer(l);
    }
}

void TileLayer::scheduleTileLoad(const Tile& tile, const CrsPtr& crs, const FeatureQuery& tileQuery)
{
    // NOTE: this method assumes that the guard has been taken
    
    // Mark tile as loading to prevent duplicate loading of the same tile
    m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, nullptr});
    m_threadPool.enqueue(
        System::ThreadPool::Task{
            .task = [this, tile, crs, tileQuery]()
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate loading time
                // FIXME: if datasets have not been initialized, the enumerator will not include all features
                auto enumerator = LayerSet::getFeatures(crs, tileQuery, true);
                
                if (enumerator->isComplete())
                {
                    double unitPerPix = tileQuery.resolution();
                    enumerator->reset();
                    enumerator = thinFeatures(enumerator, unitPerPix, tileQuery.area());
                    enumerator->reset();

                    std::lock_guard lock(m_mutex);
                    m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, enumerator});
                }
                else
                {
                    std::lock_guard lock(m_mutex);
                    m_tileManager->removeTile(tile);
                }
            },
            // NOTE: we dont acquire the lock here! This should always be called on the main thread
            .onDropped = [this, tile]() { m_tileManager->removeTile(tile); }
        }
    );
}

FeatureEnumeratorPtr TileLayer::thinFeatures(const FeatureEnumeratorPtr &features, double unitsPerPixel, const Rectangle &tileArea) const
{
    auto thinnedEnumerator = std::make_shared<FeatureEnumerator>(features->isComplete());

    for (auto& f : *features->features())
    {
        thinnedEnumerator->add(thinFeature(f, unitsPerPixel, tileArea));
    }

    for (auto& subEnum : features->subEnumerators())
    {
        thinnedEnumerator->addEnumerator(thinFeatures(subEnum, unitsPerPixel, tileArea));
    }

    return thinnedEnumerator;
}

FeaturePtr TileLayer::thinFeature(const FeaturePtr& feature, double unitsPerPixel, const Rectangle& tileArea) const
{
    switch (feature->geometryType())
    {
        case GeometryType::Point:
            return feature; // No thinning for points
        case GeometryType::Line:
        {
            auto lineGeom = feature->geometryAsLine();

            // Simple algorithm, remove every other point. This is just a placeholder, should implement an actual thinning algorithm such as Ramer-Douglas-Peucker
            auto newPoints = std::vector<Point>();
            thinLine(newPoints, lineGeom->points(), lineGeom->isClosed(), unitsPerPixel);
            auto thinnedLineGeom = std::make_shared<LineGeometry>(newPoints); // Placeholder, should implement actual thinning algorithm

            return std::make_shared<Feature>(feature->id(), feature->crs(), thinnedLineGeom, feature->attributes());
        }
        case GeometryType::Polygon:
        {
            auto polyGeom = feature->geometryAsPolygon();
            auto newPoints = std::vector<Point>();
            thinLine(newPoints, polyGeom->rings()[0], true, unitsPerPixel);
            auto thinnedPolyGeom = std::make_shared<PolygonGeometry>(newPoints); // Placeholder, should implement actual thinning algorithm

            return std::make_shared<Feature>(feature->id(), feature->crs(), thinnedPolyGeom, feature->attributes());
        }
        case GeometryType::Raster:
        {
            // Clip rasters
            // TODO: might not be needed with new FeatureQuery::resolution and FeatureQuery::rasterMode
            
            
            auto rastergeom = feature->geometryAsRaster();
            // BMM_DEBUG() << "Raster size: " << rastergeom->raster().width() << ", " << rastergeom->raster().height() << "\n";
            return feature;

            // auto subRasterGeom = rastergeom->getSubRasterGeometry(tileArea);

            // int W = subRasterGeom->raster().width();
            // double w = subRasterGeom->bounds().width();
            // double rasterUnitsPerPix = (double)w / W;

            
            // // BMM_DEBUG() << "Sub raster size: " << subRasterGeom->raster().width() << ", " << subRasterGeom->raster().height() << "\n";
            // int newW = std::round(subRasterGeom->bounds().width() / unitsPerPixel);
            // int newH = std::round(subRasterGeom->bounds().height() / unitsPerPixel);
            // // BMM_DEBUG() << "Unit per pixel: " << unitsPerPixel << "\n";
            // // BMM_DEBUG() << "Raster unit per pixel: " << rasterUnitsPerPix << "\n";
            // // BMM_DEBUG() << "Sub raster width: " << w << "\n";
            // // BMM_DEBUG() << "Sub raster WIDTH: " << W << "\n";
            // if (rasterUnitsPerPix <= unitsPerPixel)
            // {
            //     subRasterGeom->raster().resize(newW, newH);
            //     // BMM_DEBUG() << "Requested raster size: " << newW << ", " << newH << "\n";
            //     // BMM_DEBUG() << "New raster size: " << subRasterGeom->raster().width() << ", " << subRasterGeom->raster().height() << "\n";
            // }
            // else
            // {
            //     // BMM_DEBUG() << "Did not resize\n";
            // }

            // auto fakeId = Id(0, (FeatureId)subRasterGeom.get()); // FIXME: bad solution. Faking id since we only draw features with different ids
            // return std::make_shared<Feature>(feature->id(), feature->crs(), subRasterGeom, feature->attributes());
        }
        default:
            return feature; // No thinning for other geometry types for now
    }
}

void TileLayer::thinLine(std::vector<Point>& thinned, const std::vector<Point>& line, bool closed, double unitsPerPixel) const
{
    double PIXELS_PER_POINT = 3;

    thinned.reserve(line.size());
    for(size_t i=0; i<line.size(); i++)
    {
        if (thinned.empty())
        {
            // Allways add first point
            thinned.push_back(line[i]);
            continue;
        }

        if (i == line.size()-1)
        {
            // Hack
            if (closed && thinned.size() < 3)
            {
                thinned.push_back(line[i-1]);
            }
            // Allways add last point
            thinned.push_back(line[i]);
            break;
        }

        auto& prevPoint = thinned[thinned.size()-1];
        auto curr = line[i];
        double dist = (prevPoint-curr).length();
        if (dist > PIXELS_PER_POINT*unitsPerPixel)
        {
            thinned.push_back(curr);
        }
    }
}

void TileLayer::drawTiles(const MapPtr &map, const FeatureQuery &featureQuery) const
{
    // This method can be used to draw debug information about the tiles, such as their boundaries and loading status
    // For example, we could draw a rectangle for each tile, colored based on whether it's loaded, loading, or not loaded
    auto crs = map->crs();
    int tileSize = m_tileSize;
    double zoom0Resolution = crs->bounds().width() / (double)tileSize;
    double unitsPerPixel = Drawable::pixelSize() / crs->globalMetersPerUnit() / featureQuery.scale();
    int zoom = static_cast<int>(std::floor(std::log2(zoom0Resolution/unitsPerPixel)));
    zoom = std::clamp(zoom, 0, 20); // TODO: make these parameters configurable

    // BMM_DEBUG() << "TileLayer::prepare() Units per pixel: " << unitsPerPixel << ", Zoom level: " << zoom << "\n";

    map->drawable()->beginBatches();
    std::lock_guard lock(m_mutex); // Lock the whole time to prioritize rendering
    for (const Tile& tile : m_tileManager->getTilesForArea(featureQuery.area(), zoom))
    {
        Color tileColor;
        Color brushColor;
        bool hasTile = false;
        {
            hasTile = m_tileManager->hasTile(tile);
        }
        if (!hasTile)
        {
            tileColor = Color::red(1.0); // Not loaded yet
            brushColor = Color::red(0.1);
        }
        else
        {
            // If the tile is in the cache, we can add its features to the enumerator
            auto cachedTile = m_tileManager->getCachedTile(tile);
            if (cachedTile.isLoaded())
            {
                tileColor = Color::green(1.0); // Loaded
                brushColor = Color::green(0.0);
            }
            else
            {
                tileColor = Color::yellow(1.0); // Loading
                brushColor = Color::yellow(0.1);
            }
        }

        auto tileBounds = m_tileManager->tileBounds(tile.x, tile.y, tile.zoom);
        auto corners = tileBounds.corners();
        auto line = std::make_shared<LineGeometry>(corners);
        line->isClosed(true);
        auto poly = std::make_shared<PolygonGeometry>(corners);

        Pen pen(tileColor, 1.0);
        Brush brush(brushColor);
        map->drawable()->drawPolygon(poly, pen, brush);
        map->drawable()->drawLine(line, pen);
    }
    map->drawable()->endBatches();
}

void TileLayer::cleanCache()
{
    while (m_threadPool.isRunning())
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::lock_guard lock(m_mutex);
        m_tileManager = nullptr;
    }
}