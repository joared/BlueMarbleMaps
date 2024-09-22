#include "UpdateInterfaces.h"
#include "EngineObject.h"


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
    
    // FIXME: uggly fix, remove from here
    auto obj = dynamic_cast<EngineObject*>(this);
    auto child = dynamic_cast<EngineObject*>(handler);
    if (obj && child)
    {
        obj->addChild(child);
    }
    else
    { 
        std::cout << "handler must inherit EngineObject\n";
        throw std::exception();
    }
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
