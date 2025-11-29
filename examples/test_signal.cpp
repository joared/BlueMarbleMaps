#include "BlueMarbleMaps/Event/Signal.h"

#include <string>

using namespace BlueMarble;

class MySignalTester// : public ISignalHandler
{
    public:
        void slot(std::string s, int num) 
        {
            std::cout << "I am a SignalHandler: " << s << " " <<  num << "\n";
        };
};

class MySignalTester2
{
    public:
        void slot(std::string s, int num) 
        {
            std::cout << "I am an unknown instance: " << s << " " <<  num << "\n";
        };
};

void func(std::string s, int num)
{

}

int main() 
{
    int dummyCapture = 0;

    // // Test signal
    // Signal<std::string, int> mySignal;

    // // Subscription object
    // auto subscription = mySignal.createSubscription([](std::string s, int num) 
    // {
    //     std::cout << "I have a subscription: " << s << " " <<  num << "\n";
    // }
    // );
    // //subscription.cancel();

    // // Permanent/Anonymous subscription
    // mySignal += [](std::string s, int num) 
    // {
    //     std::cout << "I am anonymous: " << s << " " <<  num << "\n";
    // };

    // // Bound subscription
    // MySignalTester tester;
    // mySignal.subscribe(&tester, &MySignalTester::slot);
    // //mySignal.unsubscribe(&tester);

    // auto myLambda = [](std::string s, int num) 
    // {
    //     std::cout << "I am anonymous: " << s << " " <<  num << "\n";
    // };
    // mySignal.unsubscribe(myLambda);

    // MySignalTester2 tester2;
    // //mySignal.subscribe(&tester2, &MySignalTester2::slot);

    // for (int i(0); i<3; i++)
    // {
    //     if (i == 2)
    //     {
    //         mySignal.unsubscribe(&tester);
    //     }
    //     mySignal.notify("Emma Linder", 32+i);
    // }

    return 0;
}