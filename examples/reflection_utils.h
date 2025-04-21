#ifndef REFLECTION_UTILS
#define REFLECTION_UTILS

namespace BlueMarble
{

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
            if constexpr (refl::descriptor::has_attribute<UiProperty>(member)) 
            {
                auto&& prop = refl::descriptor::get_attribute<UiProperty>(member);
                
                auto propName = member.name.str();
                if constexpr (refl::descriptor::is_property(member))
                {
                    propName = refl::descriptor::get_display_name(member);
                }

                if (auto propIter = props.find(propName); propIter != props.end()) 
                {
                    if constexpr (refl::descriptor::is_writable(member))
                    {
                        if constexpr (refl::descriptor::is_function(member))
                        {
                            // Method setter
                            member.invoke(instance, prop.parser(propIter->second));
                        }
                        else if constexpr (refl::descriptor::is_field(member))
                        {
                            // Public field
                            member(instance) = prop.parser(propIter->second);
                        }
                    }
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
    

template <typename T>
void prettyReflectionString()
{
    constexpr auto type = refl::reflect<T>();

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
        
        std::cout << "\n";
    });
    std::cout << "\n";
    // return ""REFL_MAKE_CONST_STRING("Type: ") + typeStr + REFL_MAKE_CONST_STRING("\n") + 
    //        REFL_MAKE_CONST_STRING("Members: \n") + membersStr + 
    //        REFL_MAKE_CONST_STRING("Attributes: \n") + attributesStr;""
}

} // namespace BlueMarble

#endif /* REFLECTION_UTILS */
