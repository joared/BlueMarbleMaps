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
            virtual void renderFeature(Drawable& drawable, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) = 0;
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
            void renderFeature(Drawable& drawable, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
        protected:
            bool isValidGeometry(GeometryType type) override final;
            virtual void renderPoints(Drawable& drawable, const std::vector<Point>& points, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes) = 0;
            // Help method for converting Polygon or Line to Point feature
            void toPointFeature(FeaturePtr feature, Attributes& updateAttributes, std::vector<FeaturePtr>& outPointFeatures); // TODO: add convertGeometry?
        private:
            bool            m_isLabelOrganized;
            bool            m_atCenter;
            LabelOrganizer  m_labelOrganizer;
            
    };

    // TODO: shuold symbol visualizer handle line and polygons (instead of polygon and line having nodes)?
    // The draw back is the rendering order? Orr??? Mayvbe its betterrerreer
    class SymbolVisualizer : public PointVisualizer
    {
        public:
            SymbolVisualizer();
        protected:
            void renderPoints(Drawable& drawable, const std::vector<Point>& points, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes) override final;
    };
    typedef std::shared_ptr<SymbolVisualizer> SymbolVisualizerPtr;

    class TextVisualizer : public PointVisualizer
    {
        public:
            TextVisualizer();
            void text(const StringEvaluation& textEval);
            void backgroundColor(ColorEvaluation colorEval);
        protected:
            void renderPoints(Drawable& drawable, const std::vector<Point>& points, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes) override final;
        private:
            StringEvaluation m_textEval;
            ColorEvaluation  m_backgroundColorEval;
    };
    typedef std::shared_ptr<TextVisualizer> TextVisualizerPtr;

    class LineVisualizer : public Visualizer
    {
        public:
            LineVisualizer();
            void renderFeature(Drawable& drawable, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
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
            void renderFeature(Drawable& drawable, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
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
            void renderFeature(Drawable& drawable, FeaturePtr feature, FeaturePtr source, Attributes& updateAttributes, std::vector<PresentationObject>& presObjs) override final;
        protected:
            bool isValidGeometry(GeometryType type) override final;
    };
    typedef std::shared_ptr<RasterVisualizer> RasterVisualizerPtr;
    
}

#endif /* BLUEMARBLE_VISUALIZER */
