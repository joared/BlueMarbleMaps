#include <iostream>
#include <typeinfo>
#include <type_traits>

#include <any>
#include <vector>
#include <map>
//#define MY_REFLECTION(class, member)

class IReflection
{
    public:
        virtual const std::string& name() = 0;
        virtual const std::vector<std::string>& members() = 0;
};


class Reflection //: public IReflection
{
    public:
        template<typename T>
        static Reflection reflect()
        {
            return Reflection(typeid(T));
        }

        template<typename T>
        static Reflection reflect(const T& obj)
        {
            return Reflection(typeid(obj));
        }

        std::string name() { return m_name; };
        //const std::vector<std::string>& members() = 0;

    private:
        Reflection(const std::type_info& typeInfo, const std::vector<std::any>& members = {})
                    : m_typeInfo(typeInfo)
                    , m_name(typeInfo.name())
                    , m_members(members)
        {

        }
        const std::type_info& m_typeInfo;
        std::string m_name;
        std::vector<std::any> m_members;
};

// #define REFLECT(type, member)\
// decltype(type.#member)

class MyClass
{
    public:
        double width() { return m_width; }
        double m_width;
};

class A {};
class B { virtual void foo() {} };


/////////////// What I want to do ///////////////

// Encapsulates the data about the BleMarble object and its reflected UI element
class UIElementMetaData
{
    
};

// Stores all available UI types of UI elements that reflects BlueMarble objects (ResorceObject?)
class UIElementRegistry
{
    public:
    private:
        std::map<std::string, UIElementMetaData> m_registry;
};

// Manages UI Elements (BlueMarble objects) where connection are defined
// in a BlueMarbleLayout file
class UIElementManager
{
    public:

};

/////////////// What I want to do ///////////////

void dummyFunction() {}

void simpleTypeInfoTest()
{
    // {
    //     auto reflection = Reflection::reflect<MyClass>();
    //     std::cout << "Type reflection: " << reflection.name() << "\n";
    // }

    {
        MyClass obj;
        
        // Create variables using decltype
        {
            decltype(dummyFunction) funcType;
            decltype(MyClass::m_width) memberType;
            decltype(&MyClass::width) memberMethodType;
        }
        
        const auto& tID = typeid(&MyClass::width);
        
        auto isf = std::is_function<decltype(dummyFunction)>::value;
        auto isf2 = std::is_function<decltype(&MyClass::width)>::value;
        std::cout << "Isf: " << isf << "\n";
        std::cout << "Isf2: " << isf2 << "\n";
        std::cout << "Object method: " << tID.name() << "\n";

        auto reflection = Reflection::reflect(obj);
        std::cout << "Object reflection: " << reflection.name() << "\n";
    }
}


int main()
{
    simpleTypeInfoTest();
    // auto myObject = MyInt();
    // const auto& typeInfoObj = typeid(myObject);
    // const auto& typeInfoClass = typeid(MyInt);
    // std::cout << "Type info object: " << typeInfoObj.name() << "\n";
    // std::cout << "Type info class: " << typeInfoClass.name() << "\n";
    std::cout << std::boolalpha;
    std::cout << "Is A polymorphic? " << std::is_polymorphic<A>::value << "\n";
    std::cout << "Is B polymorphic? " << std::is_polymorphic<B>::value << "\n";
}
