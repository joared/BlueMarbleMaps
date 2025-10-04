#include "BlueMarbleMaps/System/File.h"
#include "BlueMarbleMaps/Utility/Utils.h"


using namespace BlueMarble;

std::vector<std::string> File::readLines(const std::string& filePath)
{
    return File(filePath).getLines();

}

std::string File::readAsString(const std::string &filePath)
{
    return File(filePath).asString();
}

void File::writeLines(const std::string &filePath, const std::vector<std::string> &lines)
{
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("File::writeLines() Failed to open file: " + filePath);
    }
    for (const auto& l : lines)
    {
        file << l << "\n";
    }

    file.close();
}

void File::writeString(const std::string& filePath, const std::string & string)
{
}

File::File()
    : m_filePath(""), m_file(), m_offsets(), m_step(1)
{}

File::File(const std::string &filePath)
    : m_filePath(filePath)
    , m_file()
    , m_offsets()
    , m_step(1)
{
    m_file = std::ifstream(filePath);
    if (!isOpen())
    {
        std::cerr << "Failed to open file: " << filePath << "\n";
    }
}

bool File::isOpen() const
{
    return m_file.is_open();
}

File::~File()
{
    m_file.close();
}

std::vector<std::string> File::getLines()
{
    // Check if the file is successfully opened 
    if (!isOpen()) 
    { 
        throw std::runtime_error("File::getLines() Error file not open!\n");
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(m_file, line))
    {
        lines.push_back(line);
    }

    return lines;
}

std::string File::asString()
{
    std::string s;
    for (auto& l : getLines())
    {
        s += l;
    }

    return s;
}

std::string File::getLine(size_t n)
{
    if (m_offsets.empty())
    {
        BMM_DEBUG() << "File::getLine() File empty or index not built. Call buildIndex to build index.\n";
        return "";
    }

    if (m_offsets.empty()) return {};

    // Use nearest checkpoint (for step > 1)
    size_t base = (n / m_step) * m_step;
    if (base >= m_offsets.size()) return {};

    m_file.clear();
    m_file.seekg(m_offsets[base]);

    std::string line;
    for (size_t i = base; i <= n && std::getline(m_file, line); ++i) {
        if (i == n) return line;
    }
    return {};
}

void File::buildIndex()
{
    std::string line;
    std::streampos pos = m_file.tellg();
    size_t lineNo = 0;
    while (std::getline(m_file, line)) 
    {
        if (lineNo % m_step == 0)
            m_offsets.push_back(pos);
        pos = m_file.tellg();
        ++lineNo;
    }
    m_file.clear();
    m_file.seekg(0);
}
