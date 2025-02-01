#include "BlueMarbleMaps/System/File.h"
#include "BlueMarbleMaps/Utility/Utils.h"

#include <fstream>


using namespace BlueMarble;

File::File(const std::string& filePath)
    : m_filePath(filePath)
    , m_lines(readLines(filePath))
{
    std::cout << "File()\n";
}

const std::vector<std::string> &BlueMarble::File::lines() const
{
    return m_lines;
}

std::string BlueMarble::File::asString() const
{
    std::string s;
    for (auto& l : lines())
    {
        s += l;
    }

    return s;
}

std::vector<std::string> File::readLines(const std::string& filePath)
{
    std::ifstream file(filePath);
    // Check if the file is successfully opened 
    if (!file.is_open()) 
    { 
        std::cerr << "File::readLines() Error opening the file!\n";
        return std::vector<std::string>();
    }

    std::vector<std::string> lines;
    std::string line;
    while (getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();

    return lines;
}

void File::writeLines(const std::string& filePath, const std::vector<std::string>& lines)
{
    std::ofstream file(filePath);
    // Check if the file is successfully opened 
    if (!file.is_open()) 
    { 
        std::cerr << "File::writeLines() Error opening the file!\n";
    }

    for (auto l : lines)
    {
        file << l << "\n";
    }
    
    file.close();
}
