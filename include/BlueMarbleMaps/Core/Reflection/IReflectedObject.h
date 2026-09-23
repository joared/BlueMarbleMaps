#pragma once

// Minimal reflection system.
//
//   TypeDefinition   Static description of a class: name, base class and properties.
//                    One per class, available without an instance (TypeDefinition::of<T>()).
//   PropertyDefinition  One property: name, kind, display metadata and how to get/set it on an instance.
//   Value            A property value: null, bool, int, double, string, ObjectData or a live object.
//   ObjectData       An object as plain data: a TypeDefinition + property values. Editors can show and edit
//                    these without ever instantiating the class, and ObjectData::instantiate() creates the real
//                    object (of the right, possibly derived, class) from it.
//
// Making a class reflectable:
//
//   class Shape : public IReflectedObject
//   {
//       BMM_REFLECTED(Shape, IReflectedObject)   // second argument is the reflected base class
//   public:
//       static void reflect(TypeBuilder<Shape>& t)
//       {
//           t.property("name", &Shape::name, &Shape::setName).displayName("Name");
//           t.property("visible", &Shape::m_visible);
//       }
//       ...
//   };
//
//   class Circle : public Shape
//   {
//       BMM_REFLECTED(Circle, Shape)
//   public:
//       static void reflect(TypeBuilder<Circle>& t) { t.property("radius", &Circle::m_radius).range(0, 100); }
//       ...
//   };
//
// Supported property types: bool, integers, floating point, std::string and std::shared_ptr<T> where T is a
// reflected class. Add your own type by specializing ValueTraits<T>.
//
// Type registry (not implemented yet): every TypeDefinition is reachable through T::staticType(), has a name()
// and can create() instances, so a registry is just a name -> const TypeDefinition* map. The place to fill it
// automatically is the BMM_REFLECTED macro (register from staticType()). ObjectData only holds a
// TypeDefinition pointer, so loading ObjectData from a file = registry.find(typeName) + ObjectData::set().

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <typeindex>
#include <iostream>

namespace BlueMarble {
namespace Reflection {

class IReflectedObject;
class ObjectData;
class TypeDefinition;

using ObjectPtr = std::shared_ptr<IReflectedObject>;

enum class PropertyType
{
    Bool,
    Int,
    Double,
    String,
    Object
};

// Order must match the alternatives of Value::m_value.
enum class ValueKind
{
    Null,
    Bool,
    Int,
    Double,
    String,
    Data,       // an object described as data (ObjectData)
    Instance    // a live object
};

inline std::string valueKindToString(ValueKind kind)
{
    switch (kind)
    {
        case ValueKind::Null: return "Null";
        case ValueKind::Bool: return "Bool";
        case ValueKind::Int: return "Int";
        case ValueKind::Double: return "Double";
        case ValueKind::String: return "String";
        case ValueKind::Data: return "Data";
        case ValueKind::Instance: return "Instance";
    };

    return "Unknown";
}

enum class SetResult
{
    Ok,
    UnknownProperty,
    ReadOnly,
    TypeMismatch
};

// Owning pointer with value semantics (copying copies the pointee).
// Lets ObjectData contain Values that in turn contain ObjectData.
template<typename T>
class Box
{
public:
    template<typename... Args>
    explicit Box(std::in_place_t, Args&&... args)
        : m_ptr(std::make_unique<T>(std::forward<Args>(args)...))
    {
    }
    Box(const Box& other) : m_ptr(other.m_ptr ? std::make_unique<T>(*other.m_ptr) : nullptr) {}
    Box(Box&&) noexcept = default;
    Box& operator=(const Box& other)
    {
        if (this != &other)
            m_ptr = other.m_ptr ? std::make_unique<T>(*other.m_ptr) : nullptr;
        return *this;
    }
    Box& operator=(Box&&) noexcept = default;

    T* get() { return m_ptr.get(); }
    const T* get() const { return m_ptr.get(); }

private:
    std::unique_ptr<T> m_ptr;
};

// ---------------------------------------------------------------------------------------------------------
// Value
// ---------------------------------------------------------------------------------------------------------

class Value
{
public:
    Value() = default;
    Value(std::nullptr_t) {}
    Value(bool value) : m_value(value) {}
    template<typename T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
    Value(T value) : m_value(static_cast<int64_t>(value)) {}
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    Value(T value) : m_value(static_cast<double>(value)) {}
    Value(const char* value) : m_value(std::string(value)) {}
    Value(std::string value) : m_value(std::move(value)) {}
    Value(ObjectData value);                                    // defined below, needs ObjectData
    Value(ObjectPtr value)                                      // null pointer becomes a null Value
        : m_value(value ? Variant(std::move(value)) : Variant()) {}
    template<typename U, std::enable_if_t<std::is_base_of_v<IReflectedObject, U>, int> = 0>
    Value(std::shared_ptr<U> value) : Value(ObjectPtr(std::move(value))) {}   // e.g. shared_ptr<Circle>

    ValueKind kind() const { return static_cast<ValueKind>(m_value.index()); }
    bool isNull() const { return kind() == ValueKind::Null; }

    template<typename T>
    T as() const
    {

        // return std::get<T>(m_value);
        if (auto* v = std::get_if<T>(&m_value))
            return *v;

        return T{};
    }

    // Scalars: nullopt if the value is not convertible (an Int converts to Double, a Double converts to Int
    // only if it has no fractional part).
    std::optional<bool> asBool() const
    {
        if (auto* v = std::get_if<bool>(&m_value))
            return *v;
        return std::nullopt;
    }

    std::optional<int64_t> asInt() const
    {
        if (auto* v = std::get_if<int64_t>(&m_value))
            return *v;
        if (auto* d = std::get_if<double>(&m_value))
        {
            constexpr double limit = 9223372036854775808.0; // 2^63
            if (*d >= -limit && *d < limit && *d == std::floor(*d))
                return static_cast<int64_t>(*d);
        }
        return std::nullopt;
    }

    std::optional<double> asDouble() const
    {
        if (auto* v = std::get_if<double>(&m_value))
            return *v;
        if (auto* i = std::get_if<int64_t>(&m_value))
            return static_cast<double>(*i);
        return std::nullopt;
    }

    const std::string* asString() const { return std::get_if<std::string>(&m_value); }

    // nullptr if the value is not an ObjectData / live object
    const ObjectData* asData() const
    {
        auto* box = std::get_if<Box<ObjectData>>(&m_value);
        return box ? box->get() : nullptr;
    }
    ObjectData* asData()
    {
        auto* box = std::get_if<Box<ObjectData>>(&m_value);
        return box ? box->get() : nullptr;
    }
    ObjectPtr asInstance() const
    {
        auto* p = std::get_if<ObjectPtr>(&m_value);
        return p ? *p : nullptr;
    }

private:
    using Variant = std::variant<std::monostate, bool, int64_t, double, std::string, Box<ObjectData>, ObjectPtr>;
    Variant m_value;
};

// ---------------------------------------------------------------------------------------------------------
// Instances
// ---------------------------------------------------------------------------------------------------------

class IReflectedObject
{
public:
    virtual ~IReflectedObject() = default;

    // The type of the most derived class, so this works through a base class pointer.
    // Implemented by BMM_REFLECTED.
    virtual const TypeDefinition& type() const = 0;

    // Properties of this class and all its base classes, by name.
    std::optional<Value> getProperty(const std::string& name) const;   // nullopt if there is no such property
    SetResult setProperty(const std::string& name, const Value& value);

    // This object as data. See ObjectData::describe.
    ObjectData describe() const;
};

// ---------------------------------------------------------------------------------------------------------
// Type definitions
// ---------------------------------------------------------------------------------------------------------

// Display information for editors.
struct PropertyMetadata
{
    std::string displayName;
    std::string description;

    std::optional<double> min;
    std::optional<double> max;
    std::optional<double> step;
};

class PropertyDefinition
{
public:
    const std::string& name() const { return m_name; }
    PropertyType type() const { return m_type; }
    bool readOnly() const { return !m_set; }
    const PropertyMetadata& metadata() const { return m_metadata; }

    // For Object properties, the declared type. Values can be of this type or any class derived from it.
    // nullptr for other properties.
    const TypeDefinition* objectType() const;

    // Would set() accept this value? Only checks the kind of value (and the class for objects),
    // it does not check numeric ranges.
    bool accepts(const Value& value) const;

    // Access on an instance. The object must be an instance of the class declaring the property (or a
    // subclass), otherwise get() returns a null Value and set() returns false.
    Value get(const IReflectedObject& object) const { return m_get(object); }
    bool set(IReflectedObject& object, const Value& value) const { return m_set && m_set(object, value); }

private:
    template<typename> friend class TypeBuilder;
    friend class PropertyBuilder;

    std::string m_name;
    PropertyType m_type = PropertyType::Int;
    PropertyMetadata m_metadata;
    const TypeDefinition& (*m_objectType)() = nullptr;   // a function so that a class can have properties of its own type
    std::function<Value(const IReflectedObject&)> m_get;
    std::function<bool(IReflectedObject&, const Value&)> m_set;  // empty for read-only properties
};

class OperationDefinition
{
public:
    const std::string& name() const { return m_name; }
    const std::vector<ValueKind>& parameterKinds() const { return m_arguments; }

    template<typename T, std::enable_if_t<std::is_base_of_v<IReflectedObject, T>, int> = 0>
    Value perform(T* obj, const std::vector<Value>& args) const
    {
        return m_op(obj, args);
    }
private:
    template<typename> friend class TypeBuilder;
    //friend class OperationBuilder;
    
    std::string m_name;
    std::function<Value(IReflectedObject*, const std::vector<Value>&)> m_op;
    std::vector<ValueKind> m_arguments;
};

class TypeDefinition
{
public:
    // The definition of a reflected class, without needing an instance.
    template<typename T>
    static const TypeDefinition& of() { return T::staticType(); }

    const std::string& name() const { return m_name; }
    const TypeDefinition* baseType() const { return m_base; }

    // True if this is `other` or derives from it.
    bool isA(const TypeDefinition& other) const
    {
        for (const TypeDefinition* t = this; t; t = t->m_base)
            if (t == &other)
                return true;
        return false;
    }

    // Abstract classes, and classes without a default constructor (unless a factory was given), can't be created.
    bool canCreate() const { return static_cast<bool>(m_factory); }
    ObjectPtr create() const { return m_factory ? m_factory() : nullptr; }

    // Only the properties declared by this class.
    const std::vector<PropertyDefinition>& ownProperties() const { return m_properties; }

    // All properties including inherited ones, base class properties first, in declaration order.
    // A property redeclared in a derived class replaces the base class one.
    std::vector<const PropertyDefinition*> properties() const
    {
        std::vector<const PropertyDefinition*> result;
        if (m_base)
            result = m_base->properties();
        for (const PropertyDefinition& own : m_properties)
        {
            auto same = std::find_if(result.begin(), result.end(),
                                     [&own](const PropertyDefinition* p) { return p->name() == own.name(); });
            if (same != result.end())
                *same = &own;
            else
                result.push_back(&own);
        }
        return result;
    }

    std::vector<const OperationDefinition*> operations() const
    {
        std::vector<const OperationDefinition*> result;
        if (m_base)
            result = m_base->operations();
        for (const OperationDefinition& own : m_operations)
        {
            auto same = std::find_if(result.begin(), result.end(),
                                     [&own](const OperationDefinition* p) { return p->name() == own.name(); });
            if (same != result.end())
                *same = &own;
            else
                result.push_back(&own);
        }
        return result;
    }

    // Looks in this class first, then the base classes. nullptr if not found.
    const PropertyDefinition* findProperty(const std::string& name) const
    {
        for (const TypeDefinition* t = this; t; t = t->m_base)
            for (const PropertyDefinition& p : t->m_properties)
                if (p.name() == name)
                    return &p;
        return nullptr;
    }

private:
    template<typename> friend class TypeBuilder;

    std::string m_name;
    const TypeDefinition* m_base = nullptr;
    std::vector<PropertyDefinition> m_properties;
    std::vector<OperationDefinition> m_operations;
    std::function<ObjectPtr()> m_factory;
};

// ---------------------------------------------------------------------------------------------------------
// Object data
// ---------------------------------------------------------------------------------------------------------

// An object as data: which class it is and the values of (some of) its properties.
// Properties without a value are simply not in the data. The class is never instantiated to hold this.
// ObjectData has value semantics, nested objects are copied too.
class ObjectData
{
public:
    explicit ObjectData(const TypeDefinition& type) : m_type(&type) {}

    // A live object as data, including nested objects. Read-only properties are included.
    // Since the object's own type() is used this handles derived classes correctly.
    // Cyclic references (child -> parent -> child) are cut, the second visit becomes a null value.
    static ObjectData describe(const IReflectedObject& object);

    // The values a default constructed object has. Creates a temporary object to read them, so the result
    // is empty (no values) for types that can't be created.
    static ObjectData defaults(const TypeDefinition& type);

    const TypeDefinition& type() const { return *m_type; }

    // False (and nothing changed) if the type has no such property or the value is of the wrong kind.
    bool set(const std::string& name, Value value);
    const Value* get(const std::string& name) const;
    Value* get(const std::string& name);    // to edit nested objects in place
    bool has(const std::string& name) const { return m_values.count(name) > 0; }
    void unset(const std::string& name) { m_values.erase(name); }
    const std::map<std::string, Value>& values() const { return m_values; }

    // Creates an object of this data's type (which can be a derived class of the type of the property/pointer
    // that eventually receives it) and applies the values. Null if the type can't be created or a value is
    // rejected, in which case `error` says why.
    ObjectPtr instantiate(std::string* error = nullptr) const;

    // Applies the values to an existing object, which must be of this type or a subclass. Read-only properties
    // are skipped. Continues past rejected values, returning false if there were any.
    bool applyTo(IReflectedObject& target, std::string* error = nullptr) const;

private:
    static ObjectData describe(const IReflectedObject& object, std::vector<const IReflectedObject*>& path);
    static void report(std::string* error, const std::string& message)
    {
        if (error && error->empty())
            *error = message;
    }

    const TypeDefinition* m_type;
    std::map<std::string, Value> m_values;
};

inline Value::Value(ObjectData value)
    : m_value(Box<ObjectData>(std::in_place, std::move(value)))
{
}

inline const TypeDefinition* PropertyDefinition::objectType() const
{
    return m_objectType ? &m_objectType() : nullptr;
}

inline bool PropertyDefinition::accepts(const Value& value) const
{
    switch (m_type)
    {
    case PropertyType::Bool:    return value.asBool().has_value();
    case PropertyType::Int:     return value.asInt().has_value();
    case PropertyType::Double:  return value.asDouble().has_value();
    case PropertyType::String:  return value.asString() != nullptr;
    case PropertyType::Object:
    {
        if (value.isNull())
            return true;
        const TypeDefinition* declared = objectType();
        if (const ObjectData* data = value.asData())
            return data->type().isA(*declared);
        if (ObjectPtr instance = value.asInstance())
            return instance->type().isA(*declared);
        return false;
    }
    }
    return false;
}

inline std::optional<Value> IReflectedObject::getProperty(const std::string& name) const
{
    const PropertyDefinition* property = type().findProperty(name);
    if (!property)
        return std::nullopt;
    return property->get(*this);
}

inline SetResult IReflectedObject::setProperty(const std::string& name, const Value& value)
{
    const PropertyDefinition* property = type().findProperty(name);
    if (!property)
        return SetResult::UnknownProperty;
    if (property->readOnly())
        return SetResult::ReadOnly;
    return property->set(*this, value) ? SetResult::Ok : SetResult::TypeMismatch;
}

inline ObjectData IReflectedObject::describe() const
{
    return ObjectData::describe(*this);
}

inline ObjectData ObjectData::describe(const IReflectedObject& object)
{
    std::vector<const IReflectedObject*> path;
    return describe(object, path);
}

inline ObjectData ObjectData::describe(const IReflectedObject& object, std::vector<const IReflectedObject*>& path)
{
    ObjectData data(object.type());
    path.push_back(&object);
    for (const PropertyDefinition* property : object.type().properties())
    {
        Value value = property->get(object);
        if (ObjectPtr child = value.asInstance())
        {
            bool cyclic = std::find(path.begin(), path.end(), child.get()) != path.end();
            value = cyclic ? Value() : Value(describe(*child, path));
        }
        data.m_values[property->name()] = std::move(value);
    }
    path.pop_back();
    return data;
}

inline ObjectData ObjectData::defaults(const TypeDefinition& type)
{
    if (ObjectPtr object = type.create())
        return describe(*object);
    return ObjectData(type);
}

inline bool ObjectData::set(const std::string& name, Value value)
{
    const PropertyDefinition* property = m_type->findProperty(name);
    if (!property || !property->accepts(value))
        return false;
    m_values[name] = std::move(value);
    return true;
}

inline const Value* ObjectData::get(const std::string& name) const
{
    auto it = m_values.find(name);
    return it == m_values.end() ? nullptr : &it->second;
}

inline Value* ObjectData::get(const std::string& name)
{
    auto it = m_values.find(name);
    return it == m_values.end() ? nullptr : &it->second;
}

inline ObjectPtr ObjectData::instantiate(std::string* error) const
{
    ObjectPtr object = m_type->create();
    if (!object)
    {
        report(error, "Type '" + m_type->name() + "' can't be instantiated");
        return nullptr;
    }
    if (!applyTo(*object, error))
        return nullptr;
    return object;
}

inline bool ObjectData::applyTo(IReflectedObject& target, std::string* error) const
{
    if (!target.type().isA(*m_type))
    {
        report(error, "Object of type '" + target.type().name() + "' is not a '" + m_type->name() + "'");
        return false;
    }

    bool ok = true;
    for (const auto& entry : m_values)
    {
        const PropertyDefinition* property = target.type().findProperty(entry.first);
        if (!property || property->readOnly())
            continue;
        if (!property->set(target, entry.second))
        {
            report(error, "Property '" + entry.first + "' of '" + m_type->name() + "' rejected its value");
            ok = false;
        }
    }
    return ok;
}

// ---------------------------------------------------------------------------------------------------------
// ValueTraits: how a C++ type maps to a property type and converts to/from Value.
// Specialize to support more types:
//   static constexpr PropertyType type;
//   static Value toValue(const T&);
//   static std::optional<T> fromValue(const Value&);    // nullopt if the value doesn't fit
//   static const TypeDefinition& objectType();          // only if type == PropertyType::Object
// ---------------------------------------------------------------------------------------------------------

template<typename T, typename = void>
struct ValueTraits
{
    static_assert(sizeof(T) == 0, "Unsupported property type. Specialize ValueTraits<T> to add support for it.");
};

template<>
struct ValueTraits<bool>
{
    static constexpr PropertyType type = PropertyType::Bool;
    static Value toValue(bool v) { return Value(v); }
    static std::optional<bool> fromValue(const Value& value) { return value.asBool(); }
};

template<typename T>
struct ValueTraits<T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>>
{
    static constexpr PropertyType type = PropertyType::Int;
    static Value toValue(T v) { return Value(v); }
    static std::optional<T> fromValue(const Value& value)
    {
        std::optional<int64_t> i = value.asInt();
        if (!i)
            return std::nullopt;
        if constexpr (std::is_signed_v<T>)
        {
            if (*i < static_cast<int64_t>(std::numeric_limits<T>::min()) ||
                *i > static_cast<int64_t>(std::numeric_limits<T>::max()))
                return std::nullopt;
        }
        else
        {
            if (*i < 0 || static_cast<uint64_t>(*i) > static_cast<uint64_t>(std::numeric_limits<T>::max()))
                return std::nullopt;
        }
        return static_cast<T>(*i);
    }
};

template<typename T>
struct ValueTraits<T, std::enable_if_t<std::is_floating_point_v<T>>>
{
    static constexpr PropertyType type = PropertyType::Double;
    static Value toValue(T v) { return Value(v); }
    static std::optional<T> fromValue(const Value& value)
    {
        std::optional<double> d = value.asDouble();
        if (!d)
            return std::nullopt;
        return static_cast<T>(*d);
    }
};

template<>
struct ValueTraits<std::string>
{
    static constexpr PropertyType type = PropertyType::String;
    static Value toValue(const std::string& v) { return Value(v); }
    static std::optional<std::string> fromValue(const Value& value)
    {
        const std::string* s = value.asString();
        if (!s)
            return std::nullopt;
        return *s;
    }
};

// Reflected objects. Accepts null, a live object of type U (or derived), or ObjectData of type U (or derived),
// which is instantiated on the fly.
template<typename U>
struct ValueTraits<std::shared_ptr<U>, std::enable_if_t<std::is_base_of_v<IReflectedObject, U>>>
{
    static constexpr PropertyType type = PropertyType::Object;
    static const TypeDefinition& objectType() { return U::staticType(); }
    static Value toValue(const std::shared_ptr<U>& v) { return Value(ObjectPtr(v)); }
    static std::optional<std::shared_ptr<U>> fromValue(const Value& value)
    {
        if (value.isNull())
            return std::shared_ptr<U>();

        ObjectPtr object = value.asInstance();
        if (!object)
            if (const ObjectData* data = value.asData())
                object = data->instantiate();

        std::shared_ptr<U> typed = std::dynamic_pointer_cast<U>(object);
        if (!typed)
            return std::nullopt;
        return typed;
    }
};

// ---------------------------------------------------------------------------------------------------------
// Building type definitions
// ---------------------------------------------------------------------------------------------------------

// Returned by TypeBuilder::property to describe the property for editors:
//   t.property("radius", &Circle::radius, &Circle::setRadius).displayName("Radius").range(0, 100);
class PropertyBuilder
{
public:
    PropertyBuilder(std::vector<PropertyDefinition>& properties, std::size_t index)
        : m_properties(properties), m_index(index)
    {
    }

    PropertyBuilder& displayName(std::string name) { metadata().displayName = std::move(name); return *this; }
    PropertyBuilder& description(std::string text) { metadata().description = std::move(text); return *this; }
    PropertyBuilder& range(double min, double max) { metadata().min = min; metadata().max = max; return *this; }
    PropertyBuilder& step(double step) { metadata().step = step; return *this; }

private:
    PropertyMetadata& metadata() { return m_properties[m_index].m_metadata; }

    std::vector<PropertyDefinition>& m_properties;
    std::size_t m_index;
};

template<typename T>
class TypeBuilder
{
    static_assert(std::is_base_of_v<IReflectedObject, T>, "Reflected classes must derive from IReflectedObject");

public:
    TypeBuilder(std::string name, const TypeDefinition* baseType)
    {
        m_definition.m_name = std::move(name);
        m_definition.m_base = baseType;
        if constexpr (std::is_default_constructible_v<T> && !std::is_abstract_v<T>)
            m_definition.m_factory = [] { return ObjectPtr(std::make_shared<T>()); };
    }

    // For classes that can't be default constructed.
    TypeBuilder& factory(std::function<std::shared_ptr<T>()> create)
    {
        m_definition.m_factory = [create] { return ObjectPtr(create()); };
        return *this;
    }

    // Property accessed through a getter and a setter: T get() const / void set(T). Parameter and return
    // types may also be const references. Members of base classes work as well.
    template<typename C, typename G, typename S>
    PropertyBuilder property(const std::string& name, G (C::*getter)() const, void (C::*setter)(S))
    {
        using V = std::decay_t<G>;
        static_assert(std::is_same_v<V, std::decay_t<S>>, "The getter and setter must use the same type");
        PropertyDefinition& p = addProperty<V>(name, getter);
        p.m_set = [setter](IReflectedObject& object, const Value& value) -> bool
        {
            T* self = dynamic_cast<T*>(&object);
            std::optional<V> converted = ValueTraits<V>::fromValue(value);
            if (!self || !converted)
                return false;
            (self->*setter)(std::move(*converted));
            return true;
        };
        return lastProperty();
    }

    // Read-only property.
    template<typename C, typename G>
    PropertyBuilder property(const std::string& name, G (C::*getter)() const)
    {
        addProperty<std::decay_t<G>>(name, getter);
        return lastProperty();
    }

    // Property backed directly by a data member.
    template<typename C, typename M, std::enable_if_t<!std::is_function_v<M>, int> = 0>
    PropertyBuilder property(const std::string& name, M C::*member)
    {
        using V = std::decay_t<M>;
        static_assert(!std::is_const_v<M>, "Const data members can't be reflected");
        PropertyDefinition& p = addProperty<V>(name, [member](const T& self) -> const M& { return self.*member; });
        p.m_set = [member](IReflectedObject& object, const Value& value) -> bool
        {
            T* self = dynamic_cast<T*>(&object);
            std::optional<V> converted = ValueTraits<V>::fromValue(value);
            if (!self || !converted)
                return false;
            self->*member = std::move(*converted);
            return true;
        };
        return lastProperty();
    }



    TypeDefinition build() { return std::move(m_definition); }

    template<typename R, typename... Args>
    void operation(std::string name, R (T::*method)(Args...))
    {
        OperationDefinition& opDef = m_definition.m_operations.emplace_back();
        opDef.m_name = std::move(name);
        registerArguments<Args...>(opDef);
        opDef.m_op = [method](IReflectedObject* obj, const std::vector<Value>& values) -> Value
        {
            return invoke((T*)obj, method, values, std::index_sequence_for<Args...>{});
        };
    }

    template<typename R, typename... Args>
    void operation(std::string name, std::function<R(T*, Args...)> func)
    {
        OperationDefinition& opDef = m_definition.m_operations.emplace_back();
        opDef.m_name = std::move(name);
        registerArguments<Args...>(opDef);
        opDef.m_op = [func](IReflectedObject* obj, const std::vector<Value>& values) -> Value
        {
            return invoke((T*)obj, func, values, std::index_sequence_for<Args...>{});
        };
    }

    void operation(std::string name, std::function<void(T*)> func)
    {
        OperationDefinition& opDef = m_definition.m_operations.emplace_back();
        opDef.m_name = std::move(name);
        
        opDef.m_op = [func](IReflectedObject* obj, const std::vector<Value>& values) -> Value
        {
            func((T*)obj);

            return Value{};
        };
    }

private:

    template<typename... Args>
    void registerArguments(OperationDefinition& opDef)
    {
        (opDef.m_arguments.push_back(
            Value{std::decay_t<Args>{}}.kind()
        ), ...);
    }

    template<typename A>
    static std::decay_t<A> convertArgument(const Value& value)
    {
        auto result = ValueTraits<std::decay_t<A>>::fromValue(value);

        if (!result)
            throw std::invalid_argument("Wrong argument type");

        return *result;
    }

    template<typename R, typename... Args, size_t... I>
    static Value invoke(
        T* object,
        R (T::*method)(Args...),
        const std::vector<Value>& values,
        std::index_sequence<I...>)
    {
            // Runtime check, but generated from compile-time Args...
        if (values.size() != sizeof...(Args))
            throw std::invalid_argument("Wrong number of arguments");

        if constexpr (std::is_void_v<R>)
        {
            (object->*method)(
                convertArgument<Args>(values[I])...
            );

            return Value{}; // no return value
        }
        else
        {
            Value result{
                (object->*method)(
                    convertArgument<Args>(values[I])...
                )
            };

            return result;
        }
    }

    template<typename R, typename... Args, size_t... I>
    static Value invoke(
        T* object,
        std::function<R(T*, Args...)> func,
        const std::vector<Value>& values,
        std::index_sequence<I...>)
    {
        if (values.size() != sizeof...(Args))
                throw std::invalid_argument("Wrong number of arguments");
            
        if constexpr (std::is_void_v<R>)
        {
            func((T*)object, convertArgument<Args>(values[I])...);

            return Value{}; // no return value
        }
        else
        {
            Value result = func((T*)object, convertArgument<Args>(values[I])...);

            return result;
        }
        //     // Runtime check, but generated from compile-time Args...
        // if (values.size() != sizeof...(Args))
        //     throw std::invalid_argument("Wrong number of arguments");

        // if constexpr (std::is_void_v<R>)
        // {
        //     (object->*method)(
        //         convertArgument<Args>(values[I])...
        //     );

        //     return Value{}; // no return value
        // }
        // else
        // {
        //     Value result{
        //         (object->*method)(
        //             convertArgument<Args>(values[I])...
        //         )
        //     };

        //     return result;
        // }
    }
    // `read` is a member function pointer or a callable taking the object.
    template<typename V, typename Read>
    PropertyDefinition& addProperty(const std::string& name, Read read)
    {
        PropertyDefinition& p = m_definition.m_properties.emplace_back();
        p.m_name = name;
        p.m_type = ValueTraits<V>::type;
        p.m_metadata.displayName = name;
        if constexpr (ValueTraits<V>::type == PropertyType::Object)
            p.m_objectType = &ValueTraits<V>::objectType;
        p.m_get = [read](const IReflectedObject& object) -> Value
        {
            const T* self = dynamic_cast<const T*>(&object);
            if (!self)
                return Value();
            if constexpr (std::is_member_function_pointer_v<Read>)
                return ValueTraits<V>::toValue((self->*read)());
            else
                return ValueTraits<V>::toValue(read(*self));
        };
        return p;
    }

    PropertyBuilder lastProperty()
    {
        return PropertyBuilder(m_definition.m_properties, m_definition.m_properties.size() - 1);
    }

    TypeDefinition m_definition;
};

template<typename Base>
const TypeDefinition* baseTypeOf()
{
    if constexpr (std::is_same_v<Base, IReflectedObject>)
        return nullptr;
    else
        return &Base::staticType();
}

inline std::string typeIndexToString(std::type_index kind)
{
    static const std::unordered_map<std::type_index, std::string> names = {
        {typeid(bool),       "Bool"},
        {typeid(int),        "Int"},
        {typeid(double),     "Double"},
        {typeid(std::string),"String"},
        {typeid(ObjectData), "Data"},
        {typeid(ObjectPtr),  "Instance"}
    };

    auto it = names.find(kind);
    return it != names.end() ? it->second : "Unknown";
}

} // Reflection
} // BlueMarble

// Put first in a reflected class (it leaves the class in a public: section). `Base` is the reflected base
// class, or IReflectedObject if there is none. The class must have a
//     static void reflect(BlueMarble::Reflection::TypeBuilder<Class>&)
// that lists its properties. Every class in the hierarchy needs the macro, that is what makes type() return
// the most derived type.
#define BMM_REFLECTED(Class, Base) \
public: \
    static const ::BlueMarble::Reflection::TypeDefinition& staticType() \
    { \
        static const ::BlueMarble::Reflection::TypeDefinition definition = [] \
        { \
            ::BlueMarble::Reflection::TypeBuilder<Class> builder(#Class, ::BlueMarble::Reflection::baseTypeOf<Base>()); \
            Class::reflect(builder); \
            return builder.build(); \
        }(); \
        return definition; \
    } \
    const ::BlueMarble::Reflection::TypeDefinition& type() const override { return staticType(); }
