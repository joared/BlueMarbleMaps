#include "FeatureAnimation.h"

using namespace BlueMarble;

FeatureAnimation::FeatureAnimation(FeaturePtr feature, const Point& from, const Point& to)
    : AbstractAnimation(180000, EasingFunctionType::Linear)
    , m_feature(feature)
    , m_from(from)
    , m_to(to)
{
}

void FeatureAnimation::onStarted()
{
}

void FeatureAnimation::onUpdated(double progress)
{
    auto newPos = m_from + (m_to-m_from)*progress;
    m_feature->moveTo(newPos);
}

void FeatureAnimation::onFinished()
{
    restart();
}
