#ifndef BLUEMARBLE_CSVFILE
#define BLUEMARBLE_CSVFILE

#include "BlueMarbleMaps/System/File.h"

#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <string_view>

namespace BlueMarble
{
    
    class CSVFile : public File
    {
        public:
            CSVFile(const std::string& filePath, const std::string& delimiter = ",");

            const std::vector<std::vector<std::string>>& rows();

        private:
            void extractData();
            std::vector<std::vector<std::string>>   m_rows;
            std::string                             m_delimiter;

    };

    class CSVRow
    {
        public:
            std::string_view operator[](std::size_t index) const
            {
                return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
            }
            std::size_t size() const
            {
                return m_data.size() - 1;
            }
            void readNextRow(std::istream& str)
            {
                std::getline(str, m_line);

                m_data.clear();
                m_data.emplace_back(-1);
                std::string::size_type pos = 0;
                while((pos = m_line.find(',', pos)) != std::string::npos)
                {
                    m_data.emplace_back(pos);
                    ++pos;
                }
                // This checks for a trailing comma with no data after it.
                pos   = m_line.size();
                m_data.emplace_back(pos);
            }
        private:
            std::string         m_line;
            std::vector<int>    m_data;
    };      

    std::istream& operator>>(std::istream& str, CSVRow& data);

    class CSVIterator
    {   
        public:
            typedef std::input_iterator_tag     iterator_category;
            typedef CSVRow                      value_type;
            typedef std::size_t                 difference_type;
            typedef CSVRow*                     pointer;
            typedef CSVRow&                     reference;

            CSVIterator(std::istream& str)  :m_str(str.good()?&str:nullptr) { ++(*this); }
            CSVIterator()                   :m_str(nullptr) {}

            // Pre Increment
            CSVIterator& operator++()               {if (m_str) { if (!((*m_str) >> m_row)){m_str = nullptr;}}return *this;}
            // Post increment
            CSVIterator operator++(int)             {CSVIterator    tmp(*this);++(*this);return tmp;}
            CSVRow const& operator*()   const       {return m_row;}
            CSVRow const* operator->()  const       {return &m_row;}

            bool operator==(CSVIterator const& rhs) {return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));}
            bool operator!=(CSVIterator const& rhs) {return !((*this) == rhs);}
        private:
            std::istream*       m_str;
            CSVRow              m_row;
    };
}

// int main()
// {
//     std::ifstream       file("plop.csv");

//     for(CSVIterator loop(file); loop != CSVIterator(); ++loop)
//     {
//         std::cout << "4th Element(" << (*loop)[3] << ")\n";
//     }
// }

#endif /* BLUEMARBLE_CSVFILE */
