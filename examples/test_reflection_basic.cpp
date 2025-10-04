/**
 * ***README***
 * This example illustrates an example usage scenario for
 * refl-cpp: Performing fast type-erased deserialization (from XML)
 * with the minimum possible amount of runtime overhead.
 * A custom runtime reflection system is implemented here (see UiElementMetadata).
 * It is then used to, at runtime, create instances of the reflected types and
 * initialize their properties with values read from XML.
 */
#include "refl.hpp"
#include <iostream>
#include <regex>
#include <any>
#include <functional>
#include <cassert>

// Blue marble includes
#include "BlueMarbleMaps/Core/Serialization/JsonValue.h"

#include "reflection_utils.h"

using namespace BlueMarble;

#define BMM_REFLECTION(class, ...) REFL_AUTO(type(class), __VA_ARGS__)
#define BMM_REFLECT_PROPERTY(getter, setter, name)\
func(getter, property(name)),\
func(setter, property(name))\

#define BMM_REFLECT_FIELD(name) field(name)
#define BMM_REFLECT_METHOD(name) func(name)

class SomethingElse
{
    public:
        double uno;
        double dos;
};

BMM_REFLECTION(
    SomethingElse,
    BMM_REFLECT_FIELD(uno),
    BMM_REFLECT_FIELD(dos)
);

class PointBase
{
    public:
        double getId() { return m_id; }
        double m_id = 10.0;

};

class Point : public PointBase
{
    public:
        double m_x = 1.0;
        double m_y = 2.0;
        SomethingElse m_s;
        std::vector<double> m_vectorValues;

        double getX() const
        {
            return m_x;
        }

        void setX(double value)
        {
            m_x = value;
        }

        double length() const
        {
            return std::sqrt(m_x * m_x + m_y * m_y);
        }
};

class Line
{
    public:
        std::vector<Point> m_points;
};

double parse_double(std::string_view str)
{
    return std::stod(std::string(str)); 
}

// BMM_REFLECTION
// (
//     PointBase,
//     BMM_REFLECT_FIELD(m_id),
//     BMM_REFLECT_METHOD(getId)
// );

REFL_AUTO(
    type(PointBase),      // Inheritance
    field(m_id, UiProperty(&parse_double)), // Field
    func(getId, property("idProperty"))
);



REFL_AUTO(
    type(Point, bases<PointBase>),      // Inheritance
    field(m_x, UiProperty(&parse_double)), // Field
    field(m_y),
    field(m_s),
    field(m_vectorValues),
    func(getX, property("xProperty")),  // Method with alias
    func(setX, property("xProperty"), UiProperty(&parse_double)),
    func(length)                        // Method
);

REFL_AUTO(
    type(Line),
    field(m_points)
);


class JsonParser : public JSONParseHandler
{
    public:
        JsonParser()
            : m_objects()
            , m_currObj("")
            , m_currKey("")
            , m_gotType(false)
        {}

        void onInteger(int value) override final 
        {
            //std::cout << "onInteger: " << value << "\n";
            onString(std::to_string(value));
        }

        void onDouble(double value) override final 
        {
            //std::cout << "onDouble: " << value << "\n";
            onString(std::to_string(value));
        }

        void onString(const std::string& value) override final
        {
            std::cout << "onString: " << value << "\n";

            if (m_gotType)
            {
                std::cout << "Storing object type\n";
                m_currObj = value;
                m_gotType = false;
                return;
            }

            assert(!m_currKey.empty());
            m_currProps[m_currKey] = value;
            m_currKey = "";
        }

        void onNull() override final
        {
            std::cout << "onNull" << "\n";
        }

        void onStartArray(const JsonValue::Array& value) override final 
        {
            std::cout << "onStartList: " << value.size() << "\n";
        }

        void onEndArray(const JsonValue::Array& value) override final
        {
            std::cout << "onEndList" << "\n";
        }

        void onStartObject(const JsonValue::Object& value) override final
        {
            std::cout << "onStartObject: " << value.size() << "\n";
            assert(m_currObj.empty());
        }
        
        void onEndObject(const JsonValue::Object& value) override final
        {
            std::cout << "onEndObject: " << "\n";
            if (m_currObj.empty())
            {
                std::cout << "No object type found!\n";
                throw std::exception();
            }

            m_objects[m_currObj] = m_currProps;
            m_currObj = "";
            m_gotType = false;
        }
        
        void onKey(const std::string& key) override final
        {
            std::cout << "onKey: " << key << "\n";
            assert(m_currKey.empty());
            if (key == "type")
            {
                m_gotType = true;
            }
            else
            {
                m_currKey = key;
            }
        }

    std::map<std::string, UiElementProperties> getObjects() { return m_objects; }

    private:
        std::map<std::string, UiElementProperties> m_objects;
        std::string m_currObj;
        UiElementProperties m_currProps;
        std::string m_currKey;
        bool m_gotType;
};

class LayoutParser
{
    public:
        LayoutParser()
        {
        }

        LayoutParser(const std::string& filePath)
        {
            // JSONFile f(filePath);
            // auto data = f.data();
        }


        std::map<std::string, UiElementProperties> getObjects() { return m_objects; }

        static std::map<std::string, UiElementProperties> fromString(const std::string& jsonStr)
        {
            // auto jsonData = BlueMarble::JSONFile::fromString(jsonStr);
            // auto parser = LayoutParser();
            // parser.parse(jsonData);
            // return parser.getObjects();
        }
        
    private:
        void parse(JsonValue* data)
        {
            // parseObject(data->get<JsonData>());
        }

        void parseObject(const JsonValue::Object& data)
        {
            // auto it = data.find("type");
            // if (it == data.end())
            // {
            //     std::cout << "'type' key not found\n";
            // }
            // auto nameStr = it->second->get<std::string>();
            
            // UiElementProperties props;

            // for (auto it2 : data)
            // {
            //     auto key = it2.first;
            //     auto val = it2.second;

            //     if (val->isType<std::string>())
            //     {
            //         props[key] = val->get<std::string>();
            //     }
            //     else if (val->isType<int>())
            //     {
            //         props[key] = std::to_string(val->get<int>());
            //     }
            //     else if (val->isType<double>())
            //     {
            //         props[key] = std::to_string(val->get<double>());
            //     }
            //     else if (val->isType<JsonList>())
            //     {
            //         //parseJsonList(val->get<JsonList>());
            //     }
            //     else
            //     {
            //         parseObject(val->get<JsonData>());
            //     }
            // }

            // m_objects[nameStr] = props;
        }


        std::map<std::string, UiElementProperties> m_objects;
};


int main()
{
    // 1. Print type reflection info for type
    BlueMarble::prettyReflectionString<Point>();

    // 2. Print members and their values from instance
    auto instance = Point();
    instance.m_x = 13;
    instance.m_y = 37;
    instance.m_vectorValues.push_back(1);
    instance.m_vectorValues.push_back(2);
    instance.m_vectorValues.push_back(3);
    refl::runtime::debug(std::cout, instance); // use debug_str to retrieve string
    std::cout << "\n";

    auto lineInstance = Line();
    lineInstance.m_points.push_back(instance);
    refl::runtime::debug(std::cout, lineInstance);
    std::cout << "\n";

    // 3. Invoke method using string literal
    double x = refl::runtime::invoke<double>(instance, "getX");
    std::cout << "Invoke getX result: " << x << "\n";

    // 4. Read json and instantiate
    std::string jsonStr = R"<?>(
        {
            "type" : "Point",
            "name" : "Point0",
            "xProperty" : 1338.0,
            "m_id" : 12345,
            "m_vectorValues" : [1,2,3,4]
        }
    )<?>";

    UI_ELEMENT_REGISTER(Point); // First register Point

    // Version 1
    // auto parser = JsonParser();
    // auto jsonData = BlueMarble::JSONFile::fromString(jsonStr);
    // BlueMarble::JSONFile::parseData(jsonData, &parser);
    // auto objects = parser.getObjects();
    // Version 2
    auto objects = LayoutParser::fromString(jsonStr);

    // Debug printing
    for (auto obj : objects)
    {
        std::cout << "Object: " << obj.first << "\n";
        for (auto props : obj.second)
        {
            std::cout << "\t" << props.first << ": " << props.second << "\n";
        }
    }

    // Instantiate objects
    for (auto obj : objects)
    {
        UiElementMetadata metadata = UiElementRegistry::get().find(obj.first);
        UiElementProperties props = obj.second;

        std::any element = metadata.create_instance(props);

        if (Point* sp = std::any_cast<Point>(&element)) {
            std::cout << "object at " << sp << " = ";
            refl::runtime::debug(std::cout, *sp);
        }
    }

}