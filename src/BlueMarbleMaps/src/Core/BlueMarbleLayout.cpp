#include "BlueMarbleMaps/Core/BlueMarbleLayout.h"
#include "BlueMarbleMaps/System/JsonFile.h"

using namespace BlueMarble;



BlueMarbleLayout::BlueMarbleLayout(const std::string &filePath)
{
    auto file = JSONFile(filePath);
    auto jsonValue = file.data();
    
    // Parse object
    JSONFile::parseData(jsonValue, this);
    //const auto& engineObject = parseObject(jsonValue);
}


EngineObjectPtr BlueMarbleLayout::parseObject(JsonValue *jsonValue)
{
    assert(jsonValue->isType<JsonData>());
    const auto& jsonData = jsonValue->get<JsonData>();

    auto objectName = jsonData.find("name");
    auto objectClass = jsonData.find("class");
    
    assert(objectName != jsonData.end());
    assert(objectClass != jsonData.end());

    std::string name = objectName->second->get<std::string>();
    std::string className = objectClass->second->get<std::string>();

    if (className == "MapView")
    {
        
    }
    else if (className == "Layer")
    {

    }
    else if (className == "DataSet")
    {

    }

    return EngineObjectPtr();
}