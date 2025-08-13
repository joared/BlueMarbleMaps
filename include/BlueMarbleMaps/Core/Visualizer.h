#ifndef BLUEMARBLE_VISUALIZER
#define BLUEMARBLE_VISUALIZER

#include "Feature.h"
#include "Drawable.h"
#include "PresentationObject.h"
#include "LabelOrganizer.h"
#include "AttributeVariable.h"

#include <functional>
#include <memory>
#include <vector>

namespace BlueMarble
{
    // template <typename T>
    // using FeatureEvaluation = std::function<T(FeaturePtr, Attributes&)>;

    // typedef FeatureEvaluation<bool>         Condition;
    // typedef FeatureEvaluation<Color>        ColorEvaluation;
    // typedef FeatureEvaluation<int>          IntEvaluation;
    // typedef FeatureEvaluation<double>       DoubleEvaluation;
    // typedef FeatureEvaluation<std::string>  StringEvaluation;

    class Visualizer
    {
        enum VisualizerLengtUnit
        {
            Pixels,
            Meters
        };

        public:
            Visualizer();
            void renderingEnabled(bool enabled);
            bool renderingEnabled();
            void condition(const Condition& condition);
            void color(const ColorEvaluation& colorEval);
            void offsetX(DoubleEvaluation intEval) { m_offsetXEval = intEval; }
            void offsetY(DoubleEvaluation intEval) { m_offsetYEval = intEval; }
            void size(const DoubleEvaluation& sizeEval);
            void sizeAdd(const DoubleEvaluation& sizeAddEval);
            void rotation(const DoubleEvaluation& rotationEval);
            bool attachFeature(FeaturePtr feature, FeaturePtr sourceFeature, Attributes& updateAttributes);
            void render(Drawable& drawable, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs);
            // TODO Both below should be removed and replace with renderFeatures() (to enable OpenGl stuff)
            virtual void preRender(Drawable& drawable, std::vector<FeaturePtr>& attachedFeatures, std::vector<FeaturePtr>& sourceFeatures, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) {}
            virtual void renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) = 0;
        protected:
            virtual bool isValidGeometry(GeometryType type) = 0;
            ColorEvaluation         m_colorEval;
            DoubleEvaluation        m_sizeEval;
            DoubleEvaluation        m_sizeAddEval;
            StringEvaluation        m_textEval;
            DoubleEvaluation        m_rotationEval;
            DoubleEvaluation        m_offsetXEval;
            DoubleEvaluation        m_offsetYEval;
        private:
            bool                    m_renderingEnabled;
            std::vector<FeaturePtr> m_attachedFeatures;
            std::vector<FeaturePtr> m_sourceFeatures;
            Condition               m_condition;
            VisualizerLengtUnit     m_lengthUnit;
    };
    typedef std::shared_ptr<Visualizer> VisualizerPtr;

    // Abstract base class for visualizing point geometries
    class PointVisualizer : public Visualizer
    {   
        public:
            PointVisualizer();
            void atCenter(bool atCenter) { m_atCenter = atCenter; }
            bool atCenter() { return m_atCenter; }
            bool isLabelOrganized() { return m_isLabelOrganized; }
            void isLabelOrganized(bool enabled) { m_isLabelOrganized = enabled; }

            void preRender(Drawable& drawable, std::vector<FeaturePtr>& attachedFeatures, std::vector<FeaturePtr>& sourceFeatures, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
            void renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
        protected:
            bool isValidGeometry(GeometryType type) override final;
            virtual void renderPoints(Drawable& drawable, const std::vector<Point>& points, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes) = 0;
            // Help method for converting Polygon or Line to Point feature
            void toPointFeature(const FeaturePtr& feature, Attributes& updateAttributes, std::vector<FeaturePtr>& outPointFeatures); // TODO: add convertGeometry?
        private:
            bool            m_isLabelOrganized;
            bool            m_atCenter;
            LabelOrganizer  m_labelOrganizer;
    };

    
    enum class BuiltInSymbol
    {
        Invalid,
        Circle
    };

    enum HotSpotAlignments : int
    {
        NoAlignment = 0, // Same as top left
        Top = BIT(1), 
        Bottom = BIT(2), 
        Left = BIT(3),
        Right = BIT(4),
        TopLeft = Top | Left,
        TopRight = Top | Right,
        BottomLeft = Bottom | Left,
        BottomRight = Bottom | Right,
        Center = Top | Bottom | Left | Right
    };

    // inline HotSpotAlignments operator|(HotSpotAlignments a, HotSpotAlignments b)
    // {
    //     return static_cast<HotSpotAlignments>(static_cast<int>(a) | static_cast<int>(b));
    // }

    class SymbolVisualizer : public PointVisualizer
    {
        public:
            class Symbol
            {
                public:
                    inline Symbol(BuiltInSymbol symbol=BuiltInSymbol::Circle)
                        : m_symbol(symbol)
                        , m_rasterSymbol(0,0,0,0) // Prevent warning
                        , m_hotSpot()
                        , m_rasterCache()
                    {

                    }
                    inline Symbol(const std::string& imagePath, int xHot, int yHot)
                        : m_symbol(BuiltInSymbol::Invalid)
                        , m_rasterSymbol(imagePath)
                        , m_hotSpot(xHot, yHot)
                        , m_rasterCache()
                    {

                    }

                    inline Symbol(const std::string& imagePath, HotSpotAlignments alignments)
                        : m_symbol(BuiltInSymbol::Invalid)
                        , m_rasterSymbol(imagePath)
                        , m_hotSpot(0, 0)
                        , m_rasterCache()
                    {
                        m_hotSpot = hotSpotFromAlignments(alignments);
                    }

                    inline void render(Drawable& drawable, const Point& point, double size, const Color& color, double rotation)
                    {
                        switch (m_symbol)
                        {
                        case BuiltInSymbol::Circle:
                        {
                            Brush brush;
                            brush.setColor(color);
                            drawable.drawCircle(point.x(), point.y(), size, Pen::transparent(), brush);
                            break;
                        }
                        default:
                        {
                            // Calc new width/height
                            int w = (double)m_rasterSymbol.width()*size;
                            int h = (double)m_rasterSymbol.height()*size;

                            // Use width as cache index
                            int cacheIdx = w;
                            
                            auto it = m_rasterCache.find(cacheIdx);
                            if (it == m_rasterCache.end())
                            {
                                std::cout << "Symbol::render() Created new raster cache\n";
                                auto raster = m_rasterSymbol;
                                raster.resize(w, h);
                                m_rasterCache.emplace(cacheIdx, raster);
                            }
                            int xHot = m_hotSpot.x()*size; // Only approx
                            int yHot = m_hotSpot.y()*size; // Only approx
                            auto&raster = m_rasterCache[cacheIdx];
                            if (rotation != 0)
                            {
                                // Copy and rotate the copy
                                auto raster = m_rasterCache[cacheIdx];
                                raster.rotate(rotation, xHot, yHot);
                                //did some shit...
                                //drawable.drawRaster(point.x()-xHot, point.y()-yHot, raster, color.a());
                            }
                            else 
                            {
                                // Reference and no rotation
                                auto& raster = m_rasterCache[cacheIdx];
                                //did some shit...
                                //drawable.drawRaster(point.x()-xHot, point.y()-yHot, raster, color.a());
                            }
                            
                            //auto img = *static_cast<cimg_library::CImg<unsigned char>*>(m_rasterSymbol.data());
                            //img.draw_image(x, y, rasterImg.get_shared_channels(0,2), rasterImg.get_shared_channel(3), 1.0, 255);
                            break;
                        }
                        }
                    }
                private:
                    inline Point hotSpotFromAlignments(HotSpotAlignments alignments)
                    {
                        int xHot, yHot = 0;
                        int w = m_rasterSymbol.width();
                        int h = m_rasterSymbol.height();
                        if (alignments & HotSpotAlignments::Center)
                        {
                            return Point((double)w*0.5, (double)h*0.5);
                        }

                        xHot = alignments & Left ? 0 : xHot;
                        xHot = alignments & Right ? w-1 : xHot;
                        yHot = alignments & Top ? 0 : yHot;
                        yHot = alignments & Bottom ? h-1 : yHot;

                        return Point(xHot, yHot);
                    }

                    BuiltInSymbol m_symbol;
                    Raster m_rasterSymbol;
                    Point m_hotSpot;
                    std::map<int, Raster> m_rasterCache;
            };

        public:
            SymbolVisualizer();
            void symbol(const Symbol& symbol) { m_symbol = symbol; }
            const Symbol& symbol() { return m_symbol; }
        protected:
            void renderPoints(Drawable& drawable, const std::vector<Point>& points, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes) override final;
        private:
            Symbol m_symbol;
    };
    typedef std::shared_ptr<SymbolVisualizer> SymbolVisualizerPtr;

    class TextVisualizer : public PointVisualizer
    {
        public:
            TextVisualizer();
            void text(const StringEvaluation& textEval);
            void backgroundColor(ColorEvaluation colorEval);
        protected:
            void renderPoints(Drawable& drawable, const std::vector<Point>& points, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes) override final;
        private:
            StringEvaluation m_textEval;
            ColorEvaluation  m_backgroundColorEval;
    };
    typedef std::shared_ptr<TextVisualizer> TextVisualizerPtr;

    class LineVisualizer : public Visualizer
    {
        public:
            LineVisualizer();
            void renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
            void width(DoubleEvaluation widthEval);
        protected:
            bool isValidGeometry(GeometryType type) override final;
        private:
            DoubleEvaluation m_widthEval;
    };
    typedef std::shared_ptr<LineVisualizer> LineVisualizerPtr;


    class PolygonVisualizer : public Visualizer
    {
        public:
            PolygonVisualizer();
            void renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
        protected:
            bool isValidGeometry(GeometryType type) override final;
        private:
            LineVisualizerPtr   m_lineVisualizer;
            SymbolVisualizerPtr m_nodeVisualizer;
    };
    typedef std::shared_ptr<PolygonVisualizer> PolygonVisualizerPtr;


    class RasterVisualizer : public Visualizer
    {
        public:
            RasterVisualizer();
            void renderFeature(Drawable& drawable, const FeaturePtr& feature, const FeaturePtr& source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
            void alpha(const DoubleEvaluation& alphaEval);
        protected:
            bool isValidGeometry(GeometryType type) override final;
        private:
            DoubleEvaluation m_alphaEval;
    };
    typedef std::shared_ptr<RasterVisualizer> RasterVisualizerPtr;
    
}

#endif /* BLUEMARBLE_VISUALIZER */
