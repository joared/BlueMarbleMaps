#include "BlueMarbleMaps/Core/Layer/TileLayer.h"
#include "BlueMarbleMaps/Core/Map.h"


using namespace BlueMarble;


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
    return m_tileCache.at(tile.id());
}

bool TileManager::hasTile(const Tile& tile) const
{
    auto it = m_tileCache.find(tile.id());
    return it != m_tileCache.end();
}

bool TileManager::hasLoadedTile(const Tile& tile) const
{
    auto it = m_tileCache.find(tile.id());
    return it != m_tileCache.end() && it->second.isLoaded();
}

void TileManager::setTile(Tile&& tile)
{
    if (hasTile(tile))
    {
        if (m_tileCache[tile.id()].features != nullptr)
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
    m_tileCache.at(tile.id()).features = nullptr;
}

void TileManager::removeTile(const Tile &tile)
{
    m_tileCache.erase(tile.id());
}

TileLayer::TileLayer()
    : LayerSet(), m_threadPool(), m_tileManager(nullptr), m_readAsync(true)
{
    if (m_readAsync)
    {
        // TODO: make these parameters configurable
        m_threadPool.start(4, 1, System::ThreadPool::QueuePolicy::ReplaceOldestWhenFull);
        
        // TODO: add pruning of cache to prevent memory explosion
        // m_threadPool.enqueue([this]{
        //     cleanCache();
        // });
    }
}

void TileLayer::asyncRead(bool async)
{
    // TODO: must add parameters for thread pool
    throw std::runtime_error("Not implmented");

    if (m_readAsync && !async)
    {
        m_threadPool.stop();
    }
    else if (!m_readAsync && async)
    {
        m_threadPool.start(1, 1, System::ThreadPool::QueuePolicy::ReplaceOldestWhenFull);
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
    }

    if (!m_readAsync)
    {
        return LayerSet::prepare(crs, featureQuery);
    }
    
    int tileSize = 512;
    double zoom0Resolution = crs->bounds().width() / (double)tileSize;
    double unitsPerPixel = Drawable::pixelSize() / crs->globalMetersPerUnit() / featureQuery.scale();
    int zoom = static_cast<int>(std::floor(std::log2(zoom0Resolution/unitsPerPixel)));
    zoom = std::clamp(zoom, 0, 20); // TODO: make these parameters configurable
    
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
        if (!m_tileManager->hasTile(tile)) // Should be locked
        {
            // If the tile is not in the cache, we need to load it asynchronously
            auto tileQuery = featureQuery;
            tileQuery.area(m_tileManager->tileBounds(tile.x, tile.y, tile.zoom));
            tileQuery.scale(clampedScale);
            // tileQuery.resolution(zoom0Resolution / std::pow(2, zoom)); // TODO
            m_threadPool.enqueue([this, tile, crs, tileQuery, zoom0Resolution, zoom]()
            {
                {
                    BMM_DEBUG() << "Waiting...\n";
                    std::lock_guard lock(m_mutex);
                    if (m_tileManager->hasLoadedTile(tile)) // Its loaded or loading, we can return
                    {
                        BMM_DEBUG() << "Nothing to do...\n";
                        return;
                    }

                    
                    // Placeholder tile to mark it as loading
                    auto dummy = std::make_shared<FeatureEnumerator>();
                    dummy->setFeatures(std::make_shared<FeatureCollection>());
                    if (!m_tileManager->hasTile(tile))
                    {
                        BMM_DEBUG() << "Start loading tile...\n";
                        m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, nullptr});
                    }
                    else
                    {
                        BMM_DEBUG() << "Reloading incomplete tile...\n";
                        //m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, dummy});
                    }
                }

                //std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // Simulate loading time
                // FIXME: if datasets have not been initialized, the enumerator will not include all features
                auto enumerator = LayerSet::getFeatures(crs, tileQuery, true);
                
                if (enumerator->isComplete())
                {
                    double unitPerPix = zoom0Resolution / std::pow(2.0, zoom);
                    enumerator = thinFeatures(enumerator, unitPerPix, tileQuery.area());
                    enumerator->reset();
                    std::lock_guard lock(m_mutex);
                    // m_tileManager->markTileDirty(tile);
                    m_tileManager->setTile(Tile{tile.x, tile.y, tile.zoom, enumerator});
                }
                else
                {
                    std::lock_guard lock(m_mutex);
                    //m_tileManager->markTileDirty(tile);
                    m_tileManager->removeTile(tile);
                }
            });
        }

        if (!m_tileManager->hasLoadedTile(tile))
        {
            // Almost working
            Tile parent = tile;
            while (m_tileManager->parentOf(parent, parent))
            {
                if (m_tileManager->hasLoadedTile(parent))
                {
                    BMM_DEBUG() << "Found PARENT of " << tile.toString() << ": " << parent.toString() << "\n";
                    tile = parent;
                    tile.x = parent.x;
                    tile.y = parent.y;
                    tile.zoom = parent.zoom;
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
                BMM_DEBUG() << "Preparing tile: " << tile.toString() << "\n";
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
    
    LayerSet::flushCache();
}

FeatureEnumeratorPtr TileLayer::thinFeatures(const FeatureEnumeratorPtr& features, double unitsPerPixel, const Rectangle& tileArea) const
{
    // This method can be used to thin the features in a tile to reduce the number of features that need to be processed and drawn
    // For example, we could implement a simple grid-based thinning algorithm that only keeps one feature per grid cell

    auto thinnedEnumerator = std::make_shared<FeatureEnumerator>(features->isComplete());

    for (auto& f : *features->features())
    {
        thinnedEnumerator->add(thinFeature(f, unitsPerPixel, tileArea));
    }

    // Placeholder implementation that just returns the original features without thinning
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
            auto rastergeom = feature->geometryAsRaster();
            auto subRasterGeom = rastergeom->getSubRasterGeometry(tileArea);

            int W = subRasterGeom->raster().width();
            int w = subRasterGeom->bounds().width();
            double uPerPix = (double)w / W;

            // BMM_DEBUG() << "unitsPerPixel: " << unitsPerPixel << "\n";
            // BMM_DEBUG() << "uPerPix: " << uPerPix << "\n";

            subRasterGeom->raster().resize(uPerPix / unitsPerPixel);

            // BMM_DEBUG() << "New raster size: " << subRasterGeom->raster().width() << ", " << subRasterGeom->raster().height() << "\n";

            auto fakeId = Id(0, (FeatureId)subRasterGeom.get()); // FIXME: bad solution. Faking id since we only draw features with different ids
            return std::make_shared<Feature>(feature->id(), feature->crs(), subRasterGeom, feature->attributes());
        }
        default:
            return feature; // No thinning for other geometry types for now
    }
}

void TileLayer::thinLine(std::vector<Point>& thinned, const std::vector<Point>& line, bool closed, double unitsPerPixel) const
{
    int PIXELS_PER_POINT = 3;

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
    int tileSize = 512;
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
            brushColor = Color::red(0.2);
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