#ifndef BLUEMARBLE_FEATUREANIMATION
#define BLUEMARBLE_FEATUREANIMATION

#include "Animation.h"
#include "Feature.h"

namespace BlueMarble
{

    class MemoryDataSet;
    typedef std::shared_ptr<MemoryDataSet> MemoryDataSetPtr;

    class FeatureAnimation : public AbstractAnimation
    {
        public:
            FeatureAnimation(FeaturePtr feature, const Point& from, const Point& to);
            FeaturePtr feature() { return m_feature; }
        protected:
            virtual void onUpdateFeature(double progress) = 0; // TODO make pure virtual
            const FeaturePtr& getFeature();
            const Point& from();
            const Point& to();
        private:
            void onStarted() override final;
            void onUpdated(double progress) override final;
            void onFinished() override final;

            MemoryDataSetPtr m_dataSet;
            FeaturePtr  m_feature;
            Point       m_from;
            Point       m_to;
    };
    typedef std::shared_ptr<FeatureAnimation> FeatureAnimationPtr;


    class SinusoidalFeatureAnimation : public FeatureAnimation
    {
        public:
            using FeatureAnimation::FeatureAnimation;
            void onUpdateFeature(double progress) override final;
            double& amplitude() { return m_amplitude; };
            double& frequency() { return m_frequency; };
        private:
            double m_amplitude = 1;
            double m_frequency = 200;
    };
    typedef std::shared_ptr<SinusoidalFeatureAnimation> SinusoidalFeatureAnimationPtr;
}

#endif /* BLUEMARBLE_FEATUREANIMATION */
