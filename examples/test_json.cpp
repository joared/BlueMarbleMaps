#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"
#include <cassert>

using namespace BlueMarble;

void test()
{
    JsonValue dd = 2;
    JsonValue json = JsonValue::Object();
    auto& data = json.get<JsonValue::Object>();

    data["bounds_simple"] = 
    {
        {std::string("xMin"), 1},
        {std::string("yMin"), 2.0},
        {std::string("xMax"), std::string("3")},
        {std::string("yMax"), false}
    };

    data["bounds"] = 
    {
        {"xMin", 1},
        {"yMin", 2.0},
        {"xMax", "3"},
        {"yMax", {{{"something", 123}, {"great\nI am great!", std::string("hello")}}}}
    };

    data["entries0"] = { 1, 2.0, "3.0", true, nullptr };
    data["entries"] = { {1,3.0}, {4,6}, nullptr };
    data["list"] = JsonValue::Array({1,2,3,4, std::string("asd")});
    data["boolean"] = true;
    data["integer"] = 12345;
    data["double"] = 12345.1337;
    data["string"] = std::string("12345");

    bool format = false;
    auto jsonString = json.toString(format);
    std::cout << "Json:\n" << jsonString << "\n";

    JsonValue parsedJson = JsonValue::fromString(jsonString);
    auto parsedString = parsedJson.toString(format);
    std::cout << "Parsed Json:\n" << parsedString << "\n";

    bool same = jsonString == parsedString;
    assert(same);
}

int main()
{
    test();
}