#include "BlueMarbleMaps/Core/Serialization/Json/JsonValue.h"
#include <fstream>
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

    JsonValue parsedJson = std::move(JsonValue::fromString(jsonString));
    auto parsedString = parsedJson.toString(format);
    std::cout << "Parsed Json:\n" << parsedString << "\n";

    bool same = jsonString == parsedString;
    if (!same) std::cerr << "FAIL: String and parsed are not equal\n";
    else std::cout << "SUCCESS: String and parsed are equal!\n";
}

void test_acceptance()
{
    const std::string dir = "/home/joar/git-repos/BlueMarbleMaps/examples/json-acceptance-tests-master/json-checker/";
    std::vector<std::pair<std::string, bool>> files{
        {"fail1.json", false},
        {"fail2.json", false},
        {"fail3.json", false},
        {"fail4.json", false},
        {"fail5.json", false},
        {"fail6.json", false},
        {"fail7.json", false},
        {"fail8.json", false},
        {"fail9.json", false},
        {"fail10.json", false},
        {"fail11.json", false},
        {"fail12.json", false},
        {"fail13.json", false},
        {"fail14.json", false},
        {"fail15.json", false},
        {"fail16.json", false},
        {"fail17.json", false},
        {"fail18.json", false},
        {"fail19.json", false},
        {"fail20.json", false},
        {"fail21.json", false},
        {"fail22.json", false},
        {"fail23.json", false},
        {"fail24.json", false},
        {"fail25.json", false},
        {"fail26.json", false},
        {"fail27.json", false},
        {"fail28.json", false},
        {"fail29.json", false},
        {"fail30.json", false},
        {"fail31.json", false},
        {"fail32.json", false},
        {"fail33.json", false},
        {"pass1.json", true},
        {"pass2.json", true},
        {"pass3.json", true},
    };
    
    std::vector<std::pair<std::string, JsonValue>> failed;
    
    for (const auto& test : files)
    {
        const auto& path = dir + test.first;
        bool shouldSucced = test.second;

        std::cout << "Testing " << test.first << ":\n";

        JsonValue json;
        auto jsonStream = std::ifstream(path);
        bool success = false;
        try
        {
            json = std::move(JsonValue::fromStream(jsonStream));
            success = json.hasValue();
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << "\n";
        }

        if (shouldSucced != success)
        {
            failed.emplace_back(std::pair{test.first, std::move(json)});
            // std::cerr << "Test " << test.first << " failed\n";
        }
    }

    
    if (!failed.empty())
    {
        std::cout << "Failed tests:\n";
        for (const auto& fail : failed)
        {
            std::cout << fail.first << ": " << fail.second.toString() << "\n";
        }
    }

    std::cout << "Test result: " << files.size()-failed.size() << "/" << files.size() << "\n";
}

int main()
{
    test();
    test_acceptance();
}