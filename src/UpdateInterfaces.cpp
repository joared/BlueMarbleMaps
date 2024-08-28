#include "UpdateInterfaces.h"

using namespace BlueMarble;

// void UpdateRequestPropagator::addUpdateHandler(IUpdateHandler* handler)
// {
// }

// void UpdateRequestPropagator::sendUpdateRequest(Map &map, const Rectangle &updateArea)
// {
// }

// void FeatureProvider::addFeatureHandler(IFeatureHandler *handler)
// {
// }

// void FeatureProvider::sendFeatures(const std::vector<FeaturePtr> &features)
// {
// }

BlueMarble::FeatureHandler::FeatureHandler()
    : m_updateHandlers()
{
}

void FeatureHandler::addUpdateHandler(IUpdateHandler *handler)
{
    m_updateHandlers.push_back(handler);
}

void FeatureHandler::sendUpdateRequest(Map &map, const Rectangle &updateArea)
{
    for (auto handler : m_updateHandlers)
    {
        handler->onUpdateRequest(map, updateArea, this);
    }
}

void FeatureHandler::sendGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
{
    for (auto handler : m_updateHandlers)
    {
        handler->onGetFeaturesRequest(attributes, features);
    }
}

FeaturePtr FeatureHandler::sendGetFeatureRequest(const Id &id)
{
    for (auto handler : m_updateHandlers)
    {
        if (auto f = handler->onGetFeatureRequest(id))
        {
            return f;
        }
    }

    return nullptr;
}
