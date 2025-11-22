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
    std::ofstream file;
    file.open(filePath);
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
    : m_filePath("")
    , m_file()
    , m_indexMutex()
    , m_offsets()
    , m_step(1)
{}

File::File(const std::string &filePath)
    : m_filePath(filePath)
    , m_file()
    , m_offsets()
    , m_step(1)
{
    m_file = std::ifstream(filePath, std::ios::in | std::ios::binary);
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
    // Try this if the below doesnt work
    // if (m_offsets.empty())
    // {
    //     BMM_DEBUG() << "File::getLine() File empty or index not built.\n";
    //     return "";
    // }

    // size_t base = (n / m_step) * m_step;
    // if (base >= m_offsets.size()) return {};

    // m_file.clear();
    // m_file.seekg(m_offsets[base], std::ios::beg);

    // std::string line;
    // for (size_t i = base; i <= n; ++i)
    // {
    //     if (!std::getline(m_file, line))
    //         return "";

    //     // Remove \r if present (Windows line endings)
    //     if (!line.empty() && line.back() == '\r')
    //         line.pop_back();

    //     if (i == n)
    //         return line;
    // }
    // return "";

    if (m_offsets.empty())
    {
        BMM_DEBUG() << "File::getLine() File empty or index not built. Call buildIndex() to build index.\n";
        return "";
    }

    // Use nearest checkpoint (for step > 1)
    size_t base = (n / m_step) * m_step;
    if (base >= m_offsets.size()) return {};

    // We want this to be thread safe.
    std::lock_guard lock(m_indexMutex);
    m_file.clear();
    m_file.seekg(m_offsets[base], std::ios::beg);

    std::string line;
    for (size_t i = base; i <= n && std::getline(m_file, line); ++i) 
    {
        if (i == n) return line;
    }
    return "";
}

void File::buildIndex()
{
    // Try this if the below doesn't work
    // m_offsets.clear();
    // m_file.clear();
    // m_file.seekg(0, std::ios::beg);

    // std::string line;
    // size_t lineNo = 0;
    // std::streampos pos;

    // while (true)
    // {
    //     pos = m_file.tellg();  // Save position at start of line
    //     if (m_file.eof()) break;

    //     if (!std::getline(m_file, line))
    //         break;

    //     if (lineNo % m_step == 0)
    //     {
    //         m_offsets.push_back(pos);  // Now correct: points to start of line
    //     }

    //     ++lineNo;
    // }

    // // Handle last line if needed
    // if (m_file.eof() && !line.empty() && lineNo % m_step == 0)
    // {
    //     m_offsets.push_back(pos);
    // }

    // m_file.clear();
    // m_file.seekg(0, std::ios::beg);


    m_offsets.clear();
    m_file.clear();
    m_file.seekg(0, std::ios::beg);

    std::string line;
    size_t lineNo = 0;

    while (true) 
    {
        std::streampos pos = m_file.tellg();  // ← Save position FIRST
        if (m_file.eof()) break;

        if (lineNo % m_step == 0) 
        {
            m_offsets.push_back(pos);
        }

        if (!std::getline(m_file, line)) {
            break;
        }

        ++lineNo;
    }

    // Optional: store final position if needed
    m_file.clear();
    m_file.seekg(0, std::ios::beg);
}
