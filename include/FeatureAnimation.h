#ifndef BLUEMARBLE_FEATUREANIMATION
#define BLUEMARBLE_FEATUREANIMATION

#include "Animation.h"
#include "Feature.h"

namespace BlueMarble
{

    class FeatureAnimation : public AbstractAnimation
    {
        public:
            FeatureAnimation(FeaturePtr feature, const Point& from, const Point& to);
            FeaturePtr feature() { return m_feature; }
        protected:
            void onStarted() override final;
            void onUpdated(double progress) override final;
            void onFinished() override final;
        private:
            FeaturePtr  m_feature;
            Point       m_from;
            Point       m_to;
    };
    typedef std::shared_ptr<FeatureAnimation> FeatureAnimationPtr;
}

#endif /* BLUEMARBLE_FEATUREANIMATION */
