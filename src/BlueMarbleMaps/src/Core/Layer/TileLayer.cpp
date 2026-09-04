#include "BlueMarbleMaps/Core/Layer/TileLayer.h"
#include "BlueMarbleMaps/Core/Layer/StandardLayer.h"
#include "BlueMarbleMaps/Core/Map.h"


using namespace BlueMarble;

#define TILELAYER_TILE_SIZE 512
#define TILELAYER_NUM_WORKERS 2//std::thread::hardware_concurrency()
#define TILELAYER_QUEUE_SIZE 2
#define TILELAYER_QUEUE_POLICY System::ThreadPool::QueuePolicy::ReplaceOldestWhenFull

TileManager::TileManager(const Rectangle& fullExtent, int tileSize)
    : m_tilingScheme(fullExtent, tileSize)
    , m_tileCache()
{
}

Rectangle TileManager::tileBounds(int x, int y, int zoom) const
{
    return m_tilingScheme.tileBounds(x, y, zoom);
}

int TileManager::tileManhattanDistance(const Tile& tile, const Point& point) const
{
    auto tx = m_tilingScheme.toTileX(point.x(), tile.zoom);
    auto ty = m_tilingScheme.toTileY(point.y(), tile.zoom);

    return std::abs(tx-tile.x) + std::abs(ty-tile.y);
}

double TileManager::zoomToResolution(int zoom) const
{
    return m_tilingScheme.zoomToResolution(zoom);
}

int TileManager::resolutionToZoom(double resolution) const
{
    return m_tilingScheme.resolutionToZoom(resolution);
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

namespace 
{
    constexpr int AlphaFadeTimeMs = 1000; // TODO: make configurable
}

TileLayer::TileLayer()
    : LayerSet()
    , m_threadPool()
    , m_numWorkers(TILELAYER_NUM_WORKERS)
    , m_queueSize(TILELAYER_QUEUE_SIZE)
    , m_tileManager(nullptr)
    , m_readAsync(true)
    , m_tileSize(TILELAYER_TILE_SIZE)
    , m_cacheAsBitmaps(true)
    , m_preloadParents(1)
    , m_tileVisualizer(std::make_shared<RasterVisualizer>())
{
    m_tileVisualizer->alpha([](FeaturePtr feature, Attributes& updateAttributes)
    {
        double alpha = feature->attributes().get<double>("__TILE_LAYER_RENDER_ALPHA", 1.0);
        int timeSinceLoadMs = updateAttributes.get<int>(UpdateAttributeKeys::UpdateTimeMs) - 
                              feature->attributes().get<int>(FeatureAttributeKeys::TileLoadTimeMs, -std::numeric_limits<int>::max());
        
        
        if (timeSinceLoadMs < AlphaFadeTimeMs)
        {
            if (timeSinceLoadMs < 0)
            {
                // FIXME: I have messed up how timestamp are handled somehow, so it could be negative
                BMM_DEBUG() << "Time since load, weird time stamps: " << timeSinceLoadMs << "\n";
                alpha = 0;
            }
            else
            {
                alpha *= (double)timeSinceLoadMs / (double)AlphaFadeTimeMs;
            }
            updateAttributes.set<bool>(UpdateAttributeKeys::UpdateRequired, true);
        }

        return alpha;
    });

    #ifdef __EMSCRIPTEN__
    m_readAsync = false; // Async reading seems to cause issues in emscripten, possibly due to the tile cache being accessed from multiple threads at the same time. Needs further investigation.
    #endif
    if (m_readAsync)
    {
        // TODO: make these parameters configurable
        //m_threadPool.start(m_numWorkers, m_queueSize, TILELAYER_QUEUE_POLICY);
        
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
        m_tileManager = std::make_unique<TileManager>(crs->bounds(), m_tileSize);
        verifyValidSubLayers();
    }

    if (!m_readAsync)
    {
        return LayerSet::prepare(crs, featureQuery);
    }
    
    if (!m_threadPool.isRunning())
    {
        // The thread pool is started in "update()" because we need a pointer to the map
        return std::make_shared<FeatureEnumerator>();
    }

    FeatureEnumeratorPtr enumerator = std::make_shared<FeatureEnumerator>();
    for (int i(0); i<layers().size(); ++i)
    {
        enumerator->addEnumerator(std::make_shared<FeatureEnumerator>());   
    }

    int zoom = m_tileManager->resolutionToZoom(featureQuery.resolution());
    std::vector<Tile> tiles = m_tileManager->getTilesForArea(featureQuery.area(), zoom);

    std::set<Tile> parentTiles;
    std::set<Tile> childTiles;

    // Used to avoid adding the same feature multiple times if it appears in multiple tiles
    std::unordered_map<Id, bool, Id::IdHash> featuresAdded;
    
    int64_t currTimeStamp = featureQuery.updateAttributes()->get<int>(UpdateAttributeKeys::UpdateTimeMs);
    std::lock_guard lock(m_mutex);
    for (Tile& tile : tiles)
    {
        if (!m_tileManager->hasTile(tile))
        {
            scheduleTileLoad(tile, crs, createTileQuery(tile, crs, featureQuery));

            if (m_preloadParents)
            {
                Tile parent = tile;
                int nParents = 0;
                while (nParents < m_preloadParents && m_tileManager->parentOf(parent, parent))
                {
                    ++nParents;
                    if (!m_tileManager->hasTile(parent))
                    {
                        auto parentQuery = std::move(createTileQuery(parent, crs, featureQuery));
                        scheduleTileLoad(parent, crs, parentQuery);
                    }
                }
            }
        }

        bool tileNotLoaded = m_cacheAsBitmaps
                             && !m_tileManager->hasLoadedTile(tile);

        bool isTileFadingIn = false;
        if (m_tileManager->hasLoadedTile(tile))
        {
            const auto& cachedTile = m_tileManager->getCachedTile(tile);
            isTileFadingIn = (currTimeStamp - cachedTile.timestamp < AlphaFadeTimeMs);
        }

        if (tileNotLoaded || isTileFadingIn)
        {   
            // Walk up the hierarchy to find the nearest fully-opaque loaded parent
            bool isParentFadingIn;
            auto pTiles = findLoadedParentsOf(tile, currTimeStamp, isParentFadingIn);
            if (!pTiles.empty())
            {
                parentTiles.insert(pTiles.begin(), pTiles.end());
            }
            
            if (isParentFadingIn)
            {
                // FInd loaded child tiles
                int maxDepth = 5;
                auto cTiles = findLoadedChildrenOf(tile, maxDepth, currTimeStamp);

                // We can use "parentTiles" since its empty anyway
                childTiles.insert(cTiles.begin(), cTiles.end());
            }

        }

        if (m_tileManager->hasLoadedTile(tile))
        {
            // If the tile is in the cache, we can add its features to the enumerator
            
            auto cachedTile = m_tileManager->getCachedTile(tile);

            if (cachedTile.isLoaded())
            {   
                // BMM_DEBUG() << "Preparing tile: " << tile.toString() << "\n";
                if (m_cacheAsBitmaps)
                {
                    if (cachedTile.cachedBitmapFeature)
                    {
                        enumerator->add(cachedTile.cachedBitmapFeature);
                    }
                }
                else
                {
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
    }

    if (m_cacheAsBitmaps)
    {
        FeatureEnumeratorPtr parentEnumerator = std::make_shared<FeatureEnumerator>();
        parentEnumerator->addEnumerator(enumerator);
        
        for (auto t : childTiles)
        {
            // FIXME: this is to potentially detect the null bug in update
            if (!t.cachedBitmapFeature)
            {
                throw std::runtime_error("Tried to add a child tile with cachedBitmapFeature=null");
            }
            parentEnumerator->add(t.cachedBitmapFeature);
        }

        for (auto t : parentTiles)
        {
            // FIXME: this is to potentially detect the null bug in update
            if (!t.cachedBitmapFeature)
            {
                throw std::runtime_error("Tried to add a parent tile with cachedBitmapFeature=null");
            }
            parentEnumerator->add(t.cachedBitmapFeature);
        }

        return parentEnumerator;
    }
    return enumerator;
}

void TileLayer::update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery)
{
    if (!m_currentMainMap)
    {
        m_currentMainMap = map;
        if (!m_threadPool.isRunning())
        {
            m_threadPool.start(m_numWorkers, m_queueSize, TILELAYER_QUEUE_POLICY);
        }
    }

    if (!m_cacheAsBitmaps)
    {
        LayerSet::update(map, features, featureQuery);
    }
    else
    {
        map->drawable()->beginBatches();
        while (features->moveNext())
        {
            m_tileVisualizer->renderFeature(
                *map->drawable(), 
                features->current(), // Sometimes this is null, thats no good!
                map->updateAttributes(),
                featureQuery.area());
        }
        map->drawable()->endBatches();
    }

    bool drawDebugTiles = false; // TODO: make this configurable
    if (drawDebugTiles && !layers().empty())
    {
        drawTiles(map, featureQuery);
    }
}

void TileLayer::flushCache()
{
    m_threadPool.stop(true);
    {
        std::lock_guard lock(m_mutex);
        m_tileManager = nullptr;
    }

    if (m_currentMainMap)
    {
        // m_offscreenDrawable->makeCurrent();
        // m_offscreenDrawable->flushCache();
        // m_offscreenDrawable = nullptr;
        m_currentMainMap = nullptr;
    }
    
    LayerSet::flushCache();
}

void BlueMarble::TileLayer::setCachePath(const std::string& path)
{
    m_cachePath = path;
}

void TileLayer::setNumWorkers(int nWorkers) 
{
    m_numWorkers = nWorkers;
    flushCache();
}

void TileLayer::setTileSize(int tileSize)
{
    m_tileSize = tileSize;
    flushCache();
}

void TileLayer::setQueueSize(int queueSize)
{
    m_queueSize = queueSize;
    flushCache();
}

void TileLayer::setPreloadParents(int preLoadParents)
{
    m_preloadParents = preLoadParents;
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

std::vector<Tile> TileLayer::findLoadedParentsOf(const Tile& tile, int64_t currTimeStamp, bool& isParentShowingUp)
{
    std::vector<Tile> parentTiles;
    Tile parent = tile;
    isParentShowingUp = true;
    while (m_tileManager->parentOf(parent, parent))
    {
        if (!m_tileManager->hasLoadedTile(parent))
            continue;

        const auto& cachedParent = m_tileManager->getCachedTile(parent);
        isParentShowingUp = (currTimeStamp - cachedParent.timestamp < AlphaFadeTimeMs);
        
        parentTiles.push_back(cachedParent);

        if (!isParentShowingUp)
        {
            break;
        }
        // Parent is also fading in — try grandparent
    }

    return parentTiles;
}

std::vector<Tile> TileLayer::findLoadedChildrenOf(const Tile& parent, int maxDepth, int64_t currTimeStamp)
{
    if (maxDepth == 0)
    {
        return std::vector<Tile>{};
    }
    maxDepth = maxDepth-1;

    std::vector<Tile> myChildren;
    std::vector<Tile> allChildren;
    
    if (m_tileManager->childrenOf(parent, myChildren))
    {
        for (const auto& child : myChildren)
        {
            if (!m_tileManager->hasLoadedTile(child))
            {
                // Find grandchildren
                auto grandChildren = findLoadedChildrenOf(child, maxDepth, currTimeStamp);
                allChildren.insert(allChildren.end(), 
                                   grandChildren.begin(), 
                                   grandChildren.end());
                continue;
            }

            const auto& cachedChild = m_tileManager->getCachedTile(child);
            if (!cachedChild.cachedBitmapFeature)
            {
                // FIXME: remove this when bug in update is fixed
                throw std::runtime_error("The child has cachedBitmapFeature=nullptr");
            }
            allChildren.push_back(cachedChild);

            bool isChildFadingIn = (currTimeStamp - cachedChild.timestamp < AlphaFadeTimeMs);
            if (isChildFadingIn)
            {
                // Find grandchildren
                auto grandChildren = findLoadedChildrenOf(child, maxDepth, currTimeStamp);
                allChildren.insert(allChildren.end(), 
                                   grandChildren.begin(), 
                                   grandChildren.end());
            }
        }
    }

    // if (allChildren.size())
    //     BMM_DEBUG() << "Parent: " << parent.toString() << " found " << allChildren.size() << " children\n";

    return allChildren;
}

FeatureQuery TileLayer::createTileQuery(const Tile& tile, const CrsPtr& crs, const FeatureQuery& currQuery) const
{
    double unitsPerPixel = m_tileManager->zoomToResolution(tile.zoom);
    double clampedScale = Drawable::pixelSize() / crs->globalMetersPerUnit() / m_tileManager->zoomToResolution(tile.zoom);

    auto tileQuery = currQuery;

    tileQuery.area(m_tileManager->tileBounds(tile.x, tile.y, tile.zoom));
    tileQuery.scale(clampedScale);

    // TODO: Needs debugging together with ImageDataSet::onGetFeatures()
    // Its needed when crs differ, but seems slower if theyre not.
    // For datasets that dont have the same crs as requested, its unnecessary to do "clone"
    // when reqprojecting since we get a copy anyway.
    tileQuery.rasterGeometryMode(FeatureQuery::RasterGeometryMode::Clipped);
    tileQuery.resolution(unitsPerPixel);

    return tileQuery;
}

FeaturePtr TileLayer::createTileFeature(const Tile& tile, const CrsPtr& crs, Raster &&raster) const
{
    // TODO: artifaacts when very close, better solution than extending?
    //.extended(2*unitsPerPixel, 2*unitsPerPixel);
    auto rasterArea = m_tileManager->tileBounds(tile.x, tile.y, tile.zoom);
    auto rasterGeom = std::make_shared<RasterGeometry>(std::move(raster), rasterArea);
    auto rasterFeature = std::make_shared<Feature>(Id(0,tile.id()), crs, rasterGeom);
    rasterFeature->attributes().set(FeatureAttributeKeys::TileLoadTimeMs, (int)getTimeStampMs());

    return rasterFeature;
}

void TileLayer::scheduleTileLoad(const Tile &tile, const CrsPtr &crs, const FeatureQuery &tileQuery)
{
    // NOTE: this method assumes that the guard has been taken
    if (m_tileManager->hasTile(tile))
    {
        throw std::runtime_error("Tile " + tile.toString() + " already existed for some reason");
    }
    // Mark tile as loading to prevent duplicate loading of the same tile
    m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, nullptr, 0, nullptr});
    m_threadPool.enqueue(
        System::ThreadPool::Task{
            .task = [this, tile, crs, tileQuery]()
            {
                if (m_cacheAsBitmaps 
                    && !m_cachePath.empty()
                    && !name().empty())
                {

                    // Crate 
                    auto tilePath = m_cachePath + "/" + name() + "_" + tile.toString();

                    try
                    {
                        Raster raster(tilePath);
                        auto cachedBitmapFeature = createTileFeature(tile, crs, std::move(raster));

                        BMM_DEBUG() << "TILE LOADED FROM DISK: " << tilePath << "\n";

                        std::lock_guard lock(m_mutex);
                        m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, nullptr, getTimeStampMs(), cachedBitmapFeature});
                        return;
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                    //raster.save("tilecache/tile_" + idString + ".png");
                }
                // BMM_DEBUG() << "LOADING TILE: " << tile.toString() << "\n";
                // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate loading time
                // FIXME: if datasets have not been initialized, the enumerator will not include all features
                auto enumerator = LayerSet::getFeatures(crs, tileQuery, true);
                int nFeatures = enumerator->size();
                
                if (nFeatures == 0)
                {
                    // TODO: this might be that the layers are inactive, or a wms layer has failed a request.
                    // How do we know if we should query this again or not? For now we, remove the tile
                    // and will try to query again every update.
                    // BMM_DEBUG() << "No feature to render for " << tile.toString() << ", ignoring\n";
                    std::lock_guard lock(m_mutex);
                    m_tileManager->removeTile(tile);
                    return;
                }

                if (enumerator->isComplete())
                {
                    double unitPerPix = tileQuery.resolution();

                    enumerator->reset();
                    enumerator = thinFeatures(enumerator, unitPerPix, tileQuery.area());

                    int64_t timestamp = getTimeStampMs();
                    markFeaturesAsLoaded(enumerator, timestamp);
                    enumerator->reset();

                    auto tileCopy = tile;
                    tileCopy.features = enumerator;
                    if (m_cacheAsBitmaps)
                    {
                        auto rasterFeauture = renderTile(tileCopy, crs, tileQuery);
                        if (!rasterFeauture)
                        {
                            throw std::runtime_error("TileLayer::renderTile returned empty raster feature");
                        }

                        rasterFeauture->attributes().set(FeatureAttributeKeys::TileLoadTimeMs, (int)timestamp);
                        tileCopy.cachedBitmapFeature = rasterFeauture;
                    }

                    std::lock_guard lock(m_mutex);
                    if (m_tileManager->getCachedTile(tile).isLoaded())
                    {
                        throw std::runtime_error("Tile " + tile.toString() + " was already loaded for some reason");
                    }
                    m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, enumerator, timestamp, tileCopy.cachedBitmapFeature});
                }
                else 
                {
                    std::lock_guard lock(m_mutex);
                    m_tileManager->removeTile(tile);
                }
            },
            // NOTE: we dont acquire the lock here! This should always be called on the main thread
            .onDropped = [this, tile]() 
            { 
                if (m_tileManager->hasTile(tile))
                {
                    // BMM_DEBUG() << "DROPPED TILE: " << tile.toString() << "\n";
                    m_tileManager->removeTile(tile); 
                }
            },
            .priority = [this, tileQuery, tile] 
            {
                if (!m_currentMainMap) return 0.0;

                Point surfacePoint;
                Point dummyNormalPoint;
                auto camera = m_currentMainMap->camera();
                Ray ray = camera->ndcToWorldRay(Point(0,0));
                if (m_currentMainMap->surfaceModel()->rayIntersection(
                        ray.origin,
                        ray.direction,
                        0,
                        surfacePoint,
                        dummyNormalPoint
                    ))
                {
                    
                    return 1.0 / m_tileManager->tileManhattanDistance(tile, surfacePoint);
                }
                
                return 0.0;
            }
        }
    );
}

FeaturePtr BlueMarble::TileLayer::renderTile(const Tile& cachedTile, const CrsPtr& crs, const FeatureQuery& tileQuery)
{
    // if (!m_offscreenDrawable) return;
    /////////////////////////////////////////////////////////
    // Render on a bitma
    /////////////////////////////////////////////////////////
    auto enumerator = cachedTile.features;
    int nFeatures = enumerator->size();
    
    if (nFeatures == 1)
    {
        enumerator->moveNext();
        auto rasterFeature = enumerator->current();
        if (rasterFeature->geometryType() == GeometryType::Raster)
        {
            BMM_DEBUG() << "Already a raster feature, no need to render\n";
            

            if (m_cacheAsBitmaps 
                && !m_cachePath.empty()
                && !name().empty())
            {
                auto tilePath = m_cachePath + "/" + name() + "_" + cachedTile.toString();
                auto rasterGeom = rasterFeature->geometryAsRaster();
                rasterGeom->raster().save(tilePath);
                BMM_DEBUG() << "1STORED TILE: " << tilePath << "\n";
            }

            return rasterFeature;
        }
        else
        {
            enumerator->reset();
        }
    }

    double unitsPerPixel = m_tileManager->zoomToResolution(cachedTile.zoom);

    const auto& area = tileQuery.area();
    
    
    auto proj = OrthographicCameraProjection(m_tileSize, m_tileSize, -1, 1, unitsPerPixel).projectionMatrix();

    thread_local DrawablePtr offscreenDrawable;
    thread_local MapPtr tempMap;
    if (!offscreenDrawable)
    {
        if (!m_currentMainMap) 
        {
            throw std::runtime_error("TileLayer::renderTile() m_currentMap not set!");
        }
        auto d = m_currentMainMap->drawable();
        if (!d) 
        {
            throw std::runtime_error("TileLayer::renderTile() m_currentMap drawable is empty!");
        }
        BMM_DEBUG() << "Creating offscreen drawable for tile layer debug rendering\n";
        offscreenDrawable = d->createCompatibleOffscreenDrawable(m_tileSize, m_tileSize);
        tempMap = std::make_shared<Map>();
        tempMap->drawable(offscreenDrawable);
    }
    
    
    // 1. 
    // m_mapTempRemove = map;
    // 2.
    
    offscreenDrawable->makeCurrent();
    offscreenDrawable->resize(m_tileSize, m_tileSize);
    offscreenDrawable->clearBuffer();
    
    // Temporary: force a fixed, straight-down 2D view for this offscreen render, independent of
    // whatever the live camera is currently doing (tilt/rotation/orbit). The orthographic projection
    // above already encodes the tile's scale (unitsPerPixel); an identity view matrix means no
    // rotation/tilt gets applied on top of it, so panning/tilting the live camera shouldn't change
    // how this layer's own content looks once composited back in via blitTo.
    offscreenDrawable->setProjectionMatrix(proj);
    offscreenDrawable->setViewMatrix(glm::dmat4(1.0));
    offscreenDrawable->setRenderOrigin(Point(area.center().x(), area.center().y(), 0.0)); // must come after setViewMatrix — it resets renderOrigin to (0,0,0) as a side effect

    // Let sublayers draw on the offscreen drawable
    LayerSet::update(tempMap, cachedTile.features, tileQuery);

    auto raster = offscreenDrawable->getRaster();

    auto rasterArea = Rectangle(area.center(), unitsPerPixel*m_tileSize, unitsPerPixel*m_tileSize);
    auto rasterGeom = std::make_shared<RasterGeometry>(std::move(raster), rasterArea); // FIXME: tilequery area or tile area?
    auto rasterFeature = std::make_shared<Feature>(Id(0,cachedTile.id()), crs, rasterGeom);
    rasterFeature->attributes().set(FeatureAttributeKeys::TileLoadTimeMs, (int)getTimeStampMs());


    if (m_cacheAsBitmaps 
        && !m_cachePath.empty()
        && !name().empty())
    {
                    
        auto tilePath = m_cachePath + "/" + name() + "_" + cachedTile.toString();
        rasterGeom->raster().save(tilePath);
        BMM_DEBUG() << "2STORED TILE: " << tilePath << "\n";
    }

    return rasterFeature;
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

void BlueMarble::TileLayer::markFeaturesAsLoaded(const FeatureEnumeratorPtr &features, int64_t timestamp) const
{
    for (auto& f : *features->features())
    {
        f->attributes().set(FeatureAttributeKeys::TileLoadTimeMs, (int)timestamp);
    }

    for (auto& subEnum : features->subEnumerators())
    {
        markFeaturesAsLoaded(subEnum, timestamp);
    }
}

void TileLayer::drawTiles(const MapPtr &map, const FeatureQuery &featureQuery) const
{
    // This method can be used to draw debug information about the tiles, such as their boundaries and loading status
    // For example, we could draw a rectangle for each tile, colored based on whether it's loaded, loading, or not loaded
    int zoom = m_tileManager->resolutionToZoom(featureQuery.resolution());
    auto tiles = m_tileManager->getTilesForArea(featureQuery.area(), zoom);

    map->drawable()->beginBatches();
    std::lock_guard lock(m_mutex); // Lock the whole time to prioritize rendering
    for (const Tile& tile : tiles)
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