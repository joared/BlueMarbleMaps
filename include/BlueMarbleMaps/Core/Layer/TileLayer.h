#ifndef TILELAYER
#define TILELAYER

#include "BlueMarbleMaps/Core/Layer/LayerSet.h"
#include "BlueMarbleMaps/System/Thread.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"

namespace BlueMarble
{
    using TileId = uint64_t;
    class Tile
    {
    public:
        // Tile coordinates, e.g. XYZ or TMS
        int x;
        int y;
        int zoom;

        FeatureEnumeratorPtr features; // The features contained in this tile, could be empty if not loaded yet


        TileId id() const 
        {
            return ((uint64_t)zoom << 58) |
                    ((uint64_t)x << 29) |
                    (uint64_t)y;
         }

        bool isLoaded() const
        {
            return features != nullptr;
        }

        std::string toString() const 
        {
            std::string s;
            s += "Tile(";
            s += std::to_string(x);
            s += ", ";
            s += std::to_string(y);
            s += ", ";
            s += std::to_string(zoom);
            s += ")";

            return s;
        }
    };

    // struct TileHash
    // {
    //     std::size_t operator()(const Tile& t) const
    //     {
    //         std::size_t h = std::hash<int>()(t.x);
    //         h = h * 31 + std::hash<int>()(t.y);
    //         h = h * 31 + std::hash<int>()(t.zoom);

    //         return h;
    //     }
    // }; 

    class TilingScheme
    {
    public:
        TilingScheme(const Rectangle& fullExtent, int minZoom = 0, int maxZoom = 20)
            : m_fullExtent(fullExtent)
            , m_minZoom(minZoom)
            , m_maxZoom(maxZoom)
        {
        }

        Rectangle tileBounds(int x, int y, int zoom) const
        {
            double tileWidth = this->tileWidth(zoom);
            double tileHeight = this->tileHeight(zoom);

            double xMin = m_fullExtent.xMin() + x * tileWidth;
            double yMin = m_fullExtent.yMin() + y * tileHeight;
            double xMax = xMin + tileWidth;
            double yMax = yMin + tileHeight;

            return Rectangle(xMin, yMin, xMax, yMax);
        }

        bool parentOf(const Tile& tile, Tile& parent) const
        {
            if (tile.zoom == 0)
                return false;

            int zoom = tile.zoom - 1;
            int x = tile.x >> 1;
            int y = tile.y >> 1;

            // parent = Tile{x, y, zoom};
            parent.x = x;
            parent.y = y;
            parent.zoom = zoom;

            return true;
        }

        int tilesPerAxis(int zoom) const
        {
            return 1 << zoom;
        }

        double tileWidth(int zoom) const
        {
            return m_fullExtent.width() / tilesPerAxis(zoom);
        }

        double tileHeight(int zoom) const
        {
            return m_fullExtent.height() / tilesPerAxis(zoom);
        }

        double unitsPerPixel(int tileSize, int zoom) const
        {
            return tileWidth(zoom) / tileSize;
        }
        // Converts a geographic area to tile coordinates at a given zoom level
        std::vector<Tile> getTilesForArea(const Rectangle& area, int zoom) const
        {
            if (zoom < m_minZoom || zoom > m_maxZoom)
                return {};

            Rectangle clipped = area.intersect(m_fullExtent);
            if (clipped.isUndefined())
                return {};

            int tilesPerAxis = this->tilesPerAxis(zoom);
            double tileWidth = this->tileWidth(zoom);
            double tileHeight = this->tileHeight(zoom);

            auto toTileX = [&](double x)
            {
                return (x - m_fullExtent.xMin()) / tileWidth;
            };

            auto toTileY = [&](double y)
            {
                return (y - m_fullExtent.yMin()) / tileHeight;
            };

            int xMin = std::max(0, static_cast<int>(std::floor(toTileX(clipped.xMin()))));
            int xMax = std::min(tilesPerAxis - 1, static_cast<int>(std::ceil(toTileX(clipped.xMax()))) - 1);
            int yMin = std::max(0, static_cast<int>(std::floor(toTileY(clipped.yMin()))));
            int yMax = std::min(tilesPerAxis - 1, static_cast<int>(std::ceil(toTileY(clipped.yMax()))) - 1);

            std::vector<Tile> tiles;
            tiles.reserve((xMax - xMin + 1) * (yMax - yMin + 1));

            for (int x = xMin; x <= xMax; ++x)
                for (int y = yMin; y <= yMax; ++y)
                    tiles.push_back({x, y, zoom});

            return tiles;
        }
    private:
        Rectangle m_fullExtent; // The full geographic extent covered by the tiling scheme
        // int m_tileSize; // The size of each tile in pixels
        int m_minZoom; // The minimum zoom level supported by the tiling scheme
        int m_maxZoom; // The maximum zoom level supported by the tiling scheme
    };

    class TileManager
    {
    public:
        TileManager(const Rectangle& fullExtent);
        
        Rectangle tileBounds(int x, int y, int zoom) const;

        std::vector<Tile> getTilesForArea(const Rectangle& area, int zoom) const;

        // Fetches tile data for a given tile
        std::vector<Tile> getCachedTiles(const Rectangle& area, int zoom);
        bool hasTile(const Tile& tile) const;
        bool hasLoadedTile(const Tile& tile) const;
        const Tile& getCachedTile(const Tile& tile) const;
        void setTile(Tile&& tile);
        void removeTile(const Tile& tile);
        void markTileDirty(const Tile &tile);
        bool parentOf(const Tile& tile, Tile& parent) const
        {
            return m_tilingScheme.parentOf(tile, parent);
        }

    private:
        TilingScheme m_tilingScheme;
        std::unordered_map<std::uint64_t, Tile> m_tileCache; // Cache of loaded tiles, keyed by a hash of the tile coordinates
    };

    class TileLayer : public LayerSet
    {
    public:
        TileLayer();
        bool asyncRead() const { return m_readAsync; }
        void asyncRead(bool async);
        virtual FeatureEnumeratorPtr prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
        virtual void update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery) override final;
        virtual void flushCache() override final;
    private:
        void loadTile(Tile& tile);
        FeatureEnumeratorPtr thinFeatures(const FeatureEnumeratorPtr& features, double unitsPerPixel, const Rectangle& tileArea) const;
        FeaturePtr thinFeature(const FeaturePtr& feature, double unitsPerPixel, const Rectangle& tileArea) const;
        void thinLine(std::vector<Point>& thinned, const std::vector<Point>& line, bool closed, double unitsPerPixel) const;
        void drawTiles(const MapPtr& map, const FeatureQuery& featureQuery) const;
        void cleanCache();

        System::ThreadPool              m_threadPool;
        std::unique_ptr<TileManager>    m_tileManager;
        bool                            m_readAsync;
        mutable std::mutex              m_mutex; // Mutex for synchronizing access to the tile cache
    };

    using TileLayerPtr = std::shared_ptr<TileLayer>;

}

#endif /* TILELAYER */
