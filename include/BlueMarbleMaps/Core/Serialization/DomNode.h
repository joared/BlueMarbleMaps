#ifndef BLUEMARBLE_DOMNODE
#define BLUEMARBLE_DOMNODE

#include <variant>
#include <map>
#include <vector>
#include <string>
#include <initializer_list>

namespace BlueMarble
{
   
#define DEBUG_DOMNODE false

template<typename... Ts>
class DomNode
{
public:
    using Object = std::map<std::string, DomNode<Ts...>>;
    using Array  = std::vector<DomNode<Ts...>>;
    using Value  = std::variant<std::monostate, Object, Array, Ts...>;

    DomNode() : m_val(std::monostate{}) 
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode()\n";
    }

    DomNode(const Value& v) : m_val(v) 
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode(const Value& v)\n";
    }
    DomNode(Value&& v) : m_val(std::move(v)) 
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode(Value&& v)\n";
    }

    // Explicit constructors for Object and Array
    DomNode(const Object& o) : m_val(o) { if (DEBUG_DOMNODE) std::cout << "DomNode(const Object& o)\n"; }
    DomNode(Object&& o) : m_val(std::move(o)) { if (DEBUG_DOMNODE) std::cout << "DomNode(Object&& o)\n"; }

    DomNode(const Array& a) : m_val(a) { if (DEBUG_DOMNODE) std::cout << "DomNode(const Array& a)\n"; }
    DomNode(Array&& a) : m_val(std::move(a)) { if (DEBUG_DOMNODE) std::cout << "DomNode(Array&& a)\n"; }

    // Construct from primitive types (int, string, bool, etc.)
    template<typename T,
             typename = std::enable_if_t<(std::is_same_v<T, Ts> || ...)>>
    DomNode(T v) : m_val(std::move(v)) 
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode(T v)\n";
    }

    // Construct from object initializer list
    DomNode(std::initializer_list<std::pair<const std::string, DomNode>> init)
        : m_val(Object(init)) 
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode(std::initializer_list<std::pair<const std::string, DomNode>> init)\n";
    }

    // Construct from array initializer list
    DomNode(std::initializer_list<DomNode> init)
        : m_val(Array(init))
    {
        if (DEBUG_DOMNODE) std::cout << "DomNode(std::initializer_list<DomNode> init)\n";
    }

    bool hasValue() const { return !std::holds_alternative<std::monostate>(m_val); }
    // type queries
    template<typename T>
    bool isType() const { return std::holds_alternative<T>(m_val); }

    template<typename T>
    const T& get() const { return std::get<T>(m_val); }

    template<typename T>
    T& get() { return std::get<T>(m_val); }

    template <typename Serializer>
    std::string serialize(Serializer s)
    {
        // using T = std::decay_t<decltype(v)>

    }

    std::string toString() const
    {
        if (isType<Object>())
        {
            std::string s = "{";
            int i = 0;
            for (const auto& [key, val] : get<Object>())
            {
                s += "\"" + key + "\":";
                s += val.toString();
                if (i < get<Object>().size()-1)
                {
                    s += ",";
                }
                ++i;
            }
            s += "}";

            return s;
        }
        else if (isType<Array>())
        {
            std::string s = "[";
            int i = 0;
            for (const auto& val : get<Array>())
            {
                s += val.toString();
                if (i < get<Array>().size()-1)
                {
                    s += ",";
                }
                ++i;
            }
            s += "]";

            return s;
        }
        else if (isType<std::monostate>())
        {
            return "null";
        }
        else if (isType<bool>())
        {
            return get<bool>() ? "true" : "false";
        }
        else if (isType<int>())
        {
            return std::to_string(get<int>());
        }
        else if (isType<double>())
        {
            return std::to_string(get<double>());
        }
        else if (isType<std::string>())
        {
            return "\"" + get<std::string>() + "\"";
        }
        else
        {
            std::cout << "Unhandled DomNode value\n";
        }

        return "";
    }

private:
    Value m_val;
};

void test()
{
    typedef DomNode<bool, int, double, std::string> MyDom;
    MyDom dd = 2;
    MyDom myDom = MyDom::Object();
    auto& data = myDom.get<MyDom::Object>();

    data["bounds"] = 
    {
        {"xMin", 1},
        {"yMin", 2},
        {"xMax", 3},
        {"yMax", 4}
    };

    data["entries"] = { {1,3.0}, {4,6}, MyDom() };
    data["list"] = {1,2,3,4, std::string("asd")};
    
    std::cout << "DomNode: " << myDom.toString() << "\n";

}

}

#endif /* BLUEMARBLE_DOMNODE */
