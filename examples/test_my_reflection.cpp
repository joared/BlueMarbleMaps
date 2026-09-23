#include "BlueMarbleMaps/Core/Reflection/IReflectedObject.h"
#include "BlueMarbleMaps/Core/Map.h"
#include <iostream>
#include <string>
#include <vector>

using namespace BlueMarble::Reflection;

// ---------------------------------------------------------------------------------------------------------
// Reflected classes
// ---------------------------------------------------------------------------------------------------------

class Shape : public IReflectedObject
{
    BMM_REFLECTED(Shape, IReflectedObject)
public:
    static void reflect(TypeBuilder<Shape>& t)
    {
        t.property("name", &Shape::name, &Shape::setName).displayName("Name");
        t.property("visible", &Shape::m_visible);
    }

    virtual double area() const = 0;

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    bool m_visible = true;

private:
    std::string m_name = "unnamed";
};

class Circle : public Shape
{
    BMM_REFLECTED(Circle, Shape)
public:
    static void reflect(TypeBuilder<Circle>& t)
    {
        t.property("radius", &Circle::radius, &Circle::setRadius)
            .displayName("Radius").description("Distance from the center to the edge").range(0, 1000).step(0.5);
        t.property("area", &Circle::area);  // read-only, computed
    }

    double area() const override { return 3.14159 * m_radius * m_radius; }
    double radius() const { return m_radius; }
    void setRadius(double radius) { m_radius = radius; }

private:
    double m_radius = 1.0;
};

class Rectangle : public Shape
{
    BMM_REFLECTED(Rectangle, Shape)
public:
    static void reflect(TypeBuilder<Rectangle>& t)
    {
        t.property("width", &Rectangle::m_width);
        t.property("height", &Rectangle::m_height);
    }

    double area() const override { return m_width * m_height; }
    double m_width = 2.0;
    double m_height = 3.0;
};

// Has a polymorphic object property and a property of its own type
class Drawing : public IReflectedObject
{
    BMM_REFLECTED(Drawing, IReflectedObject)
public:
    static void reflect(TypeBuilder<Drawing>& t)
    {
        t.property("title", &Drawing::m_title);
        t.property("opacity", &Drawing::m_opacity).range(0, 255);
        t.property("shape", &Drawing::shape, &Drawing::setShape);
        t.property("parent", &Drawing::m_parent);
    }

    std::shared_ptr<Shape> shape() const { return m_shape; }
    void setShape(const std::shared_ptr<Shape>& shape) { m_shape = shape; }

    std::string m_title = "untitled";
    uint8_t m_opacity = 255;
    std::shared_ptr<Drawing> m_parent;

private:
    std::shared_ptr<Shape> m_shape;
};

// ---------------------------------------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------------------------------------

static int g_failures = 0;
#define CHECK(condition) \
    do { if (!(condition)) { ++g_failures; std::cout << "  FAILED line " << __LINE__ << ": " #condition "\n"; } } while (0)

std::string toString(const Value& value)
{
    switch (value.kind())
    {
    case ValueKind::Null:     return "null";
    case ValueKind::Bool:     return *value.asBool() ? "true" : "false";
    case ValueKind::Int:      return std::to_string(*value.asInt());
    case ValueKind::Double:   return std::to_string(*value.asDouble());
    case ValueKind::String:   return "\"" + *value.asString() + "\"";
    case ValueKind::Instance: return "<instance of " + value.asInstance()->type().name() + ">";
    case ValueKind::Data:
    {
        // What an editor would do: the type says which properties exist and how to show them,
        // the data says what their values are.
        const ObjectData& data = *value.asData();
        std::string s = data.type().name() + " { ";
        for (const PropertyDefinition* p : data.type().properties())
        {
            const Value* v = data.get(p->name());
            s += p->metadata().displayName + (p->readOnly() ? " (read-only)" : "") + " = " + (v ? toString(*v) : "<unset>") + "; ";
        }
        return s + "}";
    }
    }
    return "?";
}

// ---------------------------------------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------------------------------------

// 5. Properties of a class without instantiating it
void testStaticType()
{
    std::cout << "5. Static type information\n";
    const TypeDefinition& circle = TypeDefinition::of<Circle>();

    for (const PropertyDefinition* p : circle.properties())
    {
        std::cout << "  " << circle.name() << "." << p->name() << (p->readOnly() ? " (read-only)" : "") << "\n";
    }

    CHECK(circle.name() == "Circle");
    CHECK(circle.baseType() == &TypeDefinition::of<Shape>());
    CHECK(circle.ownProperties().size() == 2);
    CHECK(circle.properties().size() == 4);                 // name, visible (inherited), radius, area
    CHECK(circle.properties()[0]->name() == "name");        // base class first, declaration order
    CHECK(circle.properties()[2]->name() == "radius");
    CHECK(circle.findProperty("name") != nullptr);          // inherited
    CHECK(circle.findProperty("bogus") == nullptr);
    CHECK(TypeDefinition::of<Shape>().findProperty("radius") == nullptr);   // not the other way around

    const PropertyDefinition* radius = circle.findProperty("radius");
    CHECK(radius->type() == PropertyType::Double);
    CHECK(radius->metadata().displayName == "Radius");
    CHECK(radius->metadata().min == 0.0 && radius->metadata().max == 1000.0 && radius->metadata().step == 0.5);
    CHECK(circle.findProperty("area")->readOnly());
    CHECK(!radius->readOnly());

    const PropertyDefinition* shape = TypeDefinition::of<Drawing>().findProperty("shape");
    CHECK(shape->type() == PropertyType::Object);
    CHECK(shape->objectType() == &TypeDefinition::of<Shape>());
    CHECK(TypeDefinition::of<Drawing>().findProperty("parent")->objectType() == &TypeDefinition::of<Drawing>());

    CHECK(circle.canCreate());
    CHECK(!TypeDefinition::of<Shape>().canCreate());        // abstract
    CHECK(TypeDefinition::of<Shape>().create() == nullptr);
}

// 1. Get/set by name
void testGetSet()
{
    std::cout << "1. Get/set by name\n";
    Circle circle;

    CHECK(circle.setProperty("radius", 5.0) == SetResult::Ok);
    CHECK(circle.radius() == 5.0);
    CHECK(circle.getProperty("radius")->asDouble() == 5.0);

    CHECK(circle.setProperty("name", "my circle") == SetResult::Ok);        // inherited, via getter/setter
    CHECK(circle.name() == "my circle");
    CHECK(circle.setProperty("visible", false) == SetResult::Ok);           // inherited, data member
    CHECK(!circle.m_visible);

    CHECK(circle.setProperty("radius", 2) == SetResult::Ok);                // int -> double
    CHECK(circle.radius() == 2.0);

    CHECK(circle.setProperty("radius", "wide") == SetResult::TypeMismatch);
    CHECK(circle.radius() == 2.0);
    CHECK(circle.setProperty("bogus", 1) == SetResult::UnknownProperty);
    CHECK(!circle.getProperty("bogus").has_value());
    CHECK(circle.setProperty("area", 1.0) == SetResult::ReadOnly);
    CHECK(circle.getProperty("area")->asDouble() > 12.5);

    Drawing drawing;
    CHECK(drawing.setProperty("opacity", 128) == SetResult::Ok);
    CHECK(drawing.m_opacity == 128);
    CHECK(drawing.setProperty("opacity", 128.0) == SetResult::Ok);          // whole double -> int
    CHECK(drawing.setProperty("opacity", 128.5) == SetResult::TypeMismatch);
    CHECK(drawing.setProperty("opacity", 300) == SetResult::TypeMismatch);  // doesn't fit in uint8_t
    CHECK(drawing.m_opacity == 128);
}

// 2. Polymorphism
void testPolymorphism()
{
    std::cout << "2. Polymorphism\n";
    std::vector<std::shared_ptr<Shape>> shapes = {std::make_shared<Circle>(), std::make_shared<Rectangle>()};

    for (const auto& shape : shapes)
    {
        // Only knows it has a Shape, still gets the properties of the real class
        std::cout << "  " << shape->type().name() << ":";
        for (const PropertyDefinition* p : shape->type().properties())
            std::cout << " " << p->name() << "=" << toString(p->get(*shape));
        std::cout << "\n";
    }

    CHECK(shapes[0]->type().name() == "Circle");
    CHECK(shapes[1]->type().name() == "Rectangle");
    CHECK(shapes[0]->type().isA(TypeDefinition::of<Shape>()));
    CHECK(!TypeDefinition::of<Shape>().isA(TypeDefinition::of<Circle>()));
    CHECK(!shapes[1]->type().isA(TypeDefinition::of<Circle>()));
    CHECK(shapes[0]->getProperty("radius").has_value());
    CHECK(!shapes[1]->getProperty("radius").has_value());
    CHECK(shapes[1]->getProperty("width")->asDouble() == 2.0);

    // Object properties are typed by their declared class but hold any subclass
    Drawing drawing;
    CHECK(drawing.setProperty("shape", shapes[0]) == SetResult::Ok);
    CHECK(drawing.shape() == shapes[0]);
    CHECK(drawing.getProperty("shape")->asInstance() == shapes[0]);
    CHECK(drawing.setProperty("shape", shapes[1]) == SetResult::Ok);
    CHECK(drawing.setProperty("shape", nullptr) == SetResult::Ok);
    CHECK(drawing.shape() == nullptr);
    CHECK(drawing.setProperty("shape", std::make_shared<Drawing>()) == SetResult::TypeMismatch);   // not a Shape
    CHECK(drawing.setProperty("shape", 5) == SetResult::TypeMismatch);
    CHECK(drawing.setProperty("parent", shapes[0]) == SetResult::TypeMismatch);                    // not a Drawing
}

// 3. Objects as data
void testObjectData()
{
    std::cout << "3. Objects as data\n";

    // From a live object (nested objects included)
    Drawing drawing;
    auto circle = std::make_shared<Circle>();
    circle->setRadius(7);
    drawing.setShape(circle);
    drawing.m_title = "sketch";

    ObjectData data = drawing.describe();
    std::cout << "  " << toString(Value(data)) << "\n";
    CHECK(&data.type() == &TypeDefinition::of<Drawing>());
    CHECK(*data.get("title")->asString() == "sketch");
    const ObjectData* shapeData = data.get("shape")->asData();
    CHECK(shapeData && &shapeData->type() == &TypeDefinition::of<Circle>());    // the actual type, not Shape
    CHECK(shapeData->get("radius")->asDouble() == 7.0);
    CHECK(data.get("parent")->isNull());

    // Built by hand, without ever creating an object. Only what the type says is valid is accepted.
    ObjectData rect(TypeDefinition::of<Rectangle>());
    CHECK(rect.set("width", 10.0));
    CHECK(rect.set("name", "wall"));                        // inherited
    CHECK(!rect.set("radius", 1.0));                        // Circle property
    CHECK(!rect.set("width", "ten"));                       // wrong kind
    CHECK(!rect.has("height"));                             // never set
    CHECK(rect.values().size() == 2);

    // Data of the default values, for a "new object" dialog
    ObjectData defaults = ObjectData::defaults(TypeDefinition::of<Rectangle>());
    CHECK(defaults.get("height")->asDouble() == 3.0);
    CHECK(ObjectData::defaults(TypeDefinition::of<Shape>()).values().empty());   // abstract

    // Nested data can be edited in place, and copies are independent
    ObjectData copy = data;
    copy.get("shape")->asData()->set("radius", 99.0);
    CHECK(copy.get("shape")->asData()->get("radius")->asDouble() == 99.0);
    CHECK(data.get("shape")->asData()->get("radius")->asDouble() == 7.0);

    // Data accepts derived types for a property declared with a base type, and rejects others
    CHECK(data.set("shape", ObjectData(TypeDefinition::of<Rectangle>())));
    CHECK(data.set("shape", ObjectData(TypeDefinition::of<Circle>())));
    CHECK(!data.set("shape", ObjectData(TypeDefinition::of<Drawing>())));
    CHECK(data.set("shape", nullptr));

    // Cycles don't recurse forever
    auto a = std::make_shared<Drawing>();
    auto b = std::make_shared<Drawing>();
    a->m_parent = b;
    b->m_parent = a;
    ObjectData cyclic = a->describe();
    CHECK(cyclic.get("parent")->asData() != nullptr);
    CHECK(cyclic.get("parent")->asData()->get("parent")->isNull());
    a->m_parent = nullptr;  // break the cycle so the shared_ptrs are freed
    b->m_parent = nullptr;
}

// 4. Instantiating from data
void testInstantiate()
{
    std::cout << "4. Instantiate from data\n";

    ObjectData circleData(TypeDefinition::of<Circle>());
    circleData.set("radius", 4.0);
    circleData.set("name", "from data");

    ObjectPtr object = circleData.instantiate();
    CHECK(object != nullptr);
    CHECK(&object->type() == &TypeDefinition::of<Circle>());
    auto circle = std::dynamic_pointer_cast<Circle>(object);        // it really is a Circle
    CHECK(circle != nullptr);
    CHECK(circle && circle->radius() == 4.0 && circle->name() == "from data");
    CHECK(circle && circle->m_visible);                             // not in the data -> constructor default

    // Nested: a Drawing whose shape is data for a Rectangle
    ObjectData rectData(TypeDefinition::of<Rectangle>());
    rectData.set("width", 20);
    ObjectData drawingData(TypeDefinition::of<Drawing>());
    drawingData.set("title", "nested");
    drawingData.set("shape", rectData);

    auto drawing = std::dynamic_pointer_cast<Drawing>(drawingData.instantiate());
    CHECK(drawing != nullptr);
    auto rect = drawing ? std::dynamic_pointer_cast<Rectangle>(drawing->shape()) : nullptr;
    CHECK(rect != nullptr);                                         // polymorphic property got the subclass
    CHECK(rect && rect->m_width == 20.0 && rect->m_height == 3.0);

    // Round trip: live -> data -> live
    Circle original;
    original.setRadius(12);
    original.setName("round trip");
    ObjectPtr clone = original.describe().instantiate();
    CHECK(clone && clone->getProperty("radius")->asDouble() == 12.0);
    CHECK(clone && *clone->getProperty("name")->asString() == "round trip");
    CHECK(clone.get() != &original);

    // Apply data to an existing object; base class data can be applied to a subclass
    ObjectData shapeData(TypeDefinition::of<Shape>());
    shapeData.set("name", "renamed");
    CHECK(shapeData.applyTo(original));
    CHECK(original.name() == "renamed");
    Rectangle wrongType;
    CHECK(!circleData.applyTo(wrongType));

    // Failure
    std::string error;
    ObjectData abstractData(TypeDefinition::of<Shape>());
    CHECK(abstractData.instantiate(&error) == nullptr);
    std::cout << "  abstract: " << error << "\n";
    CHECK(!error.empty());

    // Value that doesn't fit (passes the kind check in ObjectData, rejected by the uint8_t setter)
    ObjectData badData(TypeDefinition::of<Drawing>());
    CHECK(badData.set("opacity", 300));
    error.clear();
    CHECK(badData.instantiate(&error) == nullptr);
    std::cout << "  out of range: " << error << "\n";
}

int main()
{
    testStaticType();
    testGetSet();
    testPolymorphism();
    testObjectData();
    testInstantiate();

    auto map = std::make_shared<BlueMarble::Map>();
    std::cout << "  " << map->type().name() << ":";
    for (const PropertyDefinition* p : map->type().properties())
        std::cout << " " << p->name() << "=" << toString(p->get(*map));
    std::cout << "\n";
    for (const OperationDefinition* op : map->type().operations())
    {
        std::vector<Value> params;
        std::cout << " " << op->name() << ": ";
        for (auto paramKind : op->parameterKinds())
        {
            if (paramKind == ValueKind::Double)
            {
                params.push_back(1337.0);
            }
                
            std::cout << valueKindToString(paramKind) << ", ";
        }

        // Make the call
        std::cout << "Calling: " << op->name() << "\n";
        std::cout << "with " << params.size() << " parameters\n";
        for (const auto& param : params)
        {
            std::cout << "before: kind = "
              << static_cast<int>(param.kind())
              << "\n";
        }
        auto val = op->perform(map.get(), params);
            std::cout << "result kind outside invoke: "
                  << static_cast<int>(val.kind()) << "\n";
        std::cout << "Calling successful!\n";
        std::cout << "Exiting!\n";
        std::cout << "\n";
    }
        
    std::cout << "\n";

    std::cout << (g_failures == 0 ? "\nAll checks passed\n" : "\nSome checks FAILED\n");
    return g_failures == 0 ? 0 : 1;
}
