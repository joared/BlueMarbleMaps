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

using UiElementProperties = std::unordered_map<std::string, std::string>;
using UiElementCreator = std::any(*)(const UiElementProperties&);

enum class UiPropertyType
{
    Default = 0b0,
    Required = 0b1,
    Content = 0b10,
    RequiredContent = Required | Content,
};

template <typename Parser>
struct UiProperty : refl::attr::usage::member
{
    const Parser parser;
    const UiPropertyType type;

    constexpr UiProperty(const Parser& parser)
        : parser(parser)
        , type(UiPropertyType::Default)
    {
    }

    constexpr UiProperty(UiPropertyType type, const Parser& parser)
        : parser(parser)
        , type(type)
    {
    }
};

UiElementProperties parse_properties(std::string str)
{
    static const std::basic_regex<char> propRegex(R"raw(\s*(\w+)="([\s\S]*)")raw");
    UiElementProperties props;

    std::smatch matches;
    while (std::regex_search(str, matches, propRegex)) {
        auto&& propName = matches[1];
        auto&& propValue = matches[2];
        props.insert({ propName.str(), propValue.str() });
        str = matches.suffix().str();
    }

    return props;
}

class UiElementMetadata
{
public:

    template <typename T>
    static UiElementMetadata create_metadata()
    {
        constexpr auto type = refl::reflect<T>();
        UiElementMetadata md;
        md.name_ = type.name.c_str();
        md.creator_ = &UiElementMetadata::create_untyped<T>;
        return md;
    }

    std::any create_instance(const UiElementProperties& props) const
    {
        return creator_(props);
    }

    std::string_view name() const
    {
        return name_;
    }

private:

    std::string name_;
    UiElementCreator creator_;

    template <typename T>
    static std::any create_untyped(const UiElementProperties& props)
    {
        T instance{};
        for_each(refl::reflect<T>().members, [&](auto member) {
            if constexpr (refl::descriptor::has_attribute<UiProperty>(member)) {
                auto&& prop = refl::descriptor::get_attribute<UiProperty>(member);
                if (auto propIter = props.find(member.name.str()); propIter != props.end()) {
                    member(instance) = prop.parser(propIter->second);
                }
            }
        });

        /**
         * for_each loop above essentially gets compiled to multiples of the following (pseudo-code):
         * if (auto propIter = props.find("MemberA"); propIter != props.end()) {
         *     instance.MemberA = prop.parser(propIter->second);
         * }
         */

        return instance;
    }

};

class UiElementRegistry
{
public:

    static UiElementRegistry& get()
    {
        static UiElementRegistry instance;
        return instance;
    }

    UiElementMetadata find(std::string_view elementName) const
    {
        auto iter = std::find_if(metadata.begin(), metadata.end(), [&](auto&& x) {
            return x.name() == elementName;
        });
        if (iter == metadata.end()) {
            throw std::runtime_error("UiElement not found");
        }
        return *iter;
    }

    template <typename T>
    void register_type()
    {
        metadata.push_back(UiElementMetadata::create_metadata<T>());
    }

private:

    std::vector<UiElementMetadata> metadata;

    UiElementRegistry() {};

};

class ParsingError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};


#define MY_CONCAT_(A, B) A##B
#define MY_CONCAT(A, B) MY_CONCAT_(A, B)
#define UI_ELEMENT_REGISTER(TypeName) static int MY_CONCAT(_global_dummy_, __COUNTER__) = (UiElementRegistry::get().register_type<TypeName>(), 0)

enum class Orientation
{
    Horizontal,
    Vertical,
};

Orientation parse_orientation(std::string_view str)
{
    if (str == "horizontal") {
        return Orientation::Horizontal;
    }
    else if (str == "vertical") {
        return Orientation::Vertical;
    }
    else {
        throw ParsingError("Cannot parse " + std::string(str) + " as Orientation");
    }
}

void debug_orientation(std::ostream& os, Orientation value)
{
    os << (value == Orientation::Horizontal ? "Horizontal" : "Vertical");
}

REFL_AUTO(type(Orientation, debug{ debug_orientation }))

struct StackPanel
{
    Orientation orientation;
    std::string content;

    static std::string parse_content(std::string_view content)
    {
        return content.data();
    }
};

REFL_AUTO(
    type(StackPanel),
    field(orientation, UiProperty(&parse_orientation)),
    field(content, UiProperty(UiPropertyType::RequiredContent, &StackPanel::parse_content))
)

UI_ELEMENT_REGISTER(StackPanel);

const char* view_template = R"<?>(
    <StackPanel orientation="horizontal"> Hello, World! </StackPanel>
    <MapView width="1337.0" height="13"> This is my width! </MapView>
)<?>";




// void debug_double(std::ostream& os, double value)
// {
//     os << "(double) " << value;
// }

// REFL_AUTO(type(double, debug{ debug_double }))

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
    private:
        double m_id = 10.0;

};

class Point : public PointBase
{
    public:
        double m_x = 1.0;
        double m_y = 2.0;
        SomethingElse m_s;

        double getX() const
        {
            return m_x;
        }

        void setX(double value)
        {
            m_y = value;
        }

        double length() const
        {
            return std::sqrt(m_x * m_x + m_y * m_y);
        }
};



BMM_REFLECTION
(
    PointBase,
    BMM_REFLECT_METHOD(getId)
);

REFL_AUTO(
    type(Point, bases<PointBase>),
    field(m_x),
    field(m_y),
    field(m_s),
    func(getX, property("xProperty")),
    func(setX, property("xProperty")),
    func(length)
);

// Same as above
// BMM_REFLECTION(
//     Point,
//     BMM_REFLECT_FIELD(m_x),
//     BMM_REFLECT_FIELD(m_y),
//     BMM_REFLECT_FIELD(m_s),
//     BMM_REFLECT_PROPERTY(getX, setX, "xProperty"),
//     BMM_REFLECT_METHOD(length)
// );

class MapView
{
    public:
        double& width() { return m_width; }
        double width() const { return m_width; }
        double height=0.0;
    private:
        double m_width;
};
typedef std::shared_ptr<MapView> MapViewPtr;

double parse_double(std::string_view str)
{
    return std::stod(std::string(str)); 
}

REFL_AUTO
(
    type(MapView),
    field(height, UiProperty(&parse_double)),
    func(width, UiProperty(&parse_double))
)

UI_ELEMENT_REGISTER(MapView);

template <typename T>
void prettyReflectionString()
{
    constexpr auto type = refl::reflect<T>();

    refl::const_string typeStr = type.name;
    refl::const_string membersStr = REFL_MAKE_CONST_STRING("");
    refl::const_string attributesStr = REFL_MAKE_CONST_STRING("");
    
    // Iterate over members
    refl::util::for_each(type.members, [](auto member, auto& membersStr) 
    {
        if constexpr (refl::descriptor::is_function(member))
        {
            std::cout << REFL_MAKE_CONST_STRING("(method) ");
            std::cout << refl::descriptor::get_name(member);

            if constexpr (refl::descriptor::is_property(member))
            {
                std::cout << " aka '" << refl::descriptor::get_display_name(member) << "'";
            }

            std::cout << ": ";

            if constexpr (refl::descriptor::is_resolved(member))
            {
                std::cout << "|Resolved|";
            }
        }
        else if constexpr (refl::descriptor::is_field(member))
        {
            std::cout << "(field) ";
            std::cout << refl::descriptor::get_name(member) << ": ";

            if constexpr (refl::descriptor::is_static(member))
            {
                std::cout << "|Static|";
            }
        }
        
        if constexpr (refl::descriptor::is_type(member))
        {
            std::cout << "|Type|";
        }
        
        if constexpr (refl::descriptor::is_readable(member))
        {
            std::cout << "|Readable|";
        }
        if constexpr (refl::descriptor::is_writable(member))
        {
            std::cout << "|Writable|";
        }
        // if constexpr (refl::descriptor::is_const(member))
        // {
        //     std::cout << "Const:";
        // }
        // if constexpr (refl::descriptor::is_resolved(member))
        // {
        //     std::cout << "Resolved:";
        // }
        // if constexpr (refl::descriptor::is_static(member))
        // {
        //     std::cout << "Static:";
        // }
        
        std::cout << "\n";
    });

    // return ""REFL_MAKE_CONST_STRING("Type: ") + typeStr + REFL_MAKE_CONST_STRING("\n") + 
    //        REFL_MAKE_CONST_STRING("Members: \n") + membersStr + 
    //        REFL_MAKE_CONST_STRING("Attributes: \n") + attributesStr;""
}

int main()
{
    std::basic_regex<char> xmlRegex(R"raw(<(\w+)([^>]*)>([\s\S]*)</\s*\1\s*>)raw");

    prettyReflectionString<Point>();

    auto instance = Point();
    instance.m_x = 13;
    instance.m_y = 37;
    refl::runtime::debug(std::cout, instance);

    std::string view(view_template);
    std::replace(view.begin(), view.end(), '\n', ' ');

    std::smatch matches;
    while (std::regex_search(view, matches, xmlRegex)) {
        if (matches.size() < 4) {
            throw std::runtime_error("Error in XML parser!");
        }

        auto&& tagName = matches[1];
        auto&& attributes = matches[2];
        auto&& content = matches[3];

        std::cout << "Matches: ";
        for (size_t i = 1; i < matches.size(); i++) {
            auto&& match = matches[i];
            std::cout << "(" << match << ")";
        }
        std::cout << '\n';

        UiElementProperties props = parse_properties(attributes.str());
        props["content"] = content;

        UiElementMetadata metadata = UiElementRegistry::get().find(tagName.str());
        std::any element = metadata.create_instance(props);

        if (StackPanel* sp = std::any_cast<StackPanel>(&element)) {
            std::cout << "object at " << sp << " = ";
            refl::runtime::debug(std::cout, *sp);
        }
        else
        if (MapView* sp = std::any_cast<MapView>(&element)) {
            std::cout << "object at " << sp << " = ";
            refl::runtime::debug(std::cout, *sp);
        }

        std::cout << '\n';
        view = matches.suffix().str();
    }

    std::cout << std::endl;
}