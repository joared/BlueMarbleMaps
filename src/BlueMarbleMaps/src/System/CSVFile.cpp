#include "BlueMarbleMaps/System/CSVFile.h"
#include "BlueMarbleMaps/Utility/Utils.h"

using namespace BlueMarble;

CSVFile::CSVFile(const std::string& filePath, const std::string& delimiter)
    : File(filePath)
    , m_rows()
    , m_delimiter(delimiter)
{
    extractData();
}

const std::vector<std::vector<std::string>> &BlueMarble::CSVFile::rows()
{
    return m_rows;
}

void CSVFile::extractData()
{
    for (auto& line : lines())
    {
        auto row = Utils::splitString(line, m_delimiter);
        m_rows.push_back(row);
    }
}

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}
