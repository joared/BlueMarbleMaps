#include "FeatureAnimation.h"
#include "DataSet.h"

using namespace BlueMarble;

FeatureAnimation::FeatureAnimation(FeaturePtr feature, const Point& from, const Point& to)
    : AbstractAnimation(180000, EasingFunctionType::Linear)
    , m_feature(feature)
    , m_from(from)
    , m_to(to)
{
    m_dataSet = std::dynamic_pointer_cast<MemoryDataSet>(DataSet::getDataSetById(m_feature->id().dataSetId()));
    assert(m_dataSet != nullptr);
}

const FeaturePtr& FeatureAnimation::getFeature()
{
    return m_feature;
}

const Point& FeatureAnimation::from()
{
    return m_from;
}

const Point& FeatureAnimation::to()
{
    return m_to;
}

void FeatureAnimation::onStarted()
{
}

void FeatureAnimation::onUpdated(double progress)
{
    onUpdateFeature(progress);
    m_dataSet->triggerFeatureUpdated(m_feature);
}

void FeatureAnimation::onFinished()
{
    restart();
}

void SinusoidalFeatureAnimation::onUpdateFeature(double progress)
{
    auto newPos = from() + (to()-from())*progress;
    auto dir = (to()-from()).norm();
    auto normVec = Point(dir.y(), -dir.x()); // Vector normal to the direction
    newPos = newPos + normVec*m_amplitude*std::sin(m_frequency*progress);

    double progressPrev = std::max(progress-0.0001, 0.0);
    auto approxPrevPos = from() + (to()-from())*progressPrev;
    auto dirPrev = (to()-from()).norm();
    auto normVecPrev = Point(dirPrev.y(), -dirPrev.x()); // Vector normal to the direction
    approxPrevPos = approxPrevPos + normVecPrev*m_amplitude*std::sin(m_frequency*progressPrev);
    
    auto approxDir = (newPos-approxPrevPos).norm();
    double rotation = atan2(approxDir.x(), approxDir.y()) * 180.0 / 3.14;

    getFeature()->moveTo(newPos);
    getFeature()->attributes().set("Rotation", rotation); // TODO: Should rotation be part of Geometry?
}
