#ifndef BLUEMARBLE_FILE
#define BLUEMARBLE_FILE

#include <iostream>
#include <vector>

namespace BlueMarble
{
    
    class File
    {
        public:
            File(const std::string& filePath);
            const std::vector<std::string>& lines() const;
            std::string asString() const;
            static std::vector<std::string> readLines(const std::string& filePath);
            static void writeLines(const std::string& filePath, const std::vector<std::string>& lines);
        protected:
            std::string m_filePath;
            std::vector<std::string> m_lines;
    };

}


#endif /* BLUEMARBLE_FILE */
