#include "EngineObject.h"

#include <iostream> // TODO: remove

using namespace BlueMarble;

BlueMarble::EngineObject::EngineObject()
    : m_name()
    , m_children()
{
}

BlueMarble::EngineObject::EngineObject(const std::string &name)
    : m_name(name)
    , m_children()
{
}

const std::string& BlueMarble::EngineObject::name()
{
    return m_name;
}

void BlueMarble::EngineObject::name(const std::string& name)
{
    m_name = name;
}

void BlueMarble::EngineObject::addChild(EngineObject* child)
{
    m_children.push_back(child);
    onChildAdded(child);
}

EngineObject* EngineObject::findChild(const std::string &name, bool recursive)
{
    for (auto c : m_children)
    {   
        if (c->name() == name)
            return c;
    }

    if (recursive)
    {
        for (auto c : m_children)
        {
            if (auto obj = c->findChild(name, true))
                return obj;
        }
    }

    return nullptr;
}
