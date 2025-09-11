#include <iostream>
#include <string>
#include <memory>

class MyClass;
typedef std::shared_ptr<MyClass> MyClassPtr;
class MyClass
    : public std::enable_shared_from_this<MyClass>
{
    public:
        MyClass () {}
        void update()
        {
            auto self = shared_from_this();
            std::cout << "Looks good!\n";
        }
};

int main() 
{
    auto obj = std::make_shared<MyClass>();
    obj->update();

    return 0;
}