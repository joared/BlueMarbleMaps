#ifndef BLUEMARBLE_FILE
#define BLUEMARBLE_FILE

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>

namespace BlueMarble
{
    class File2
    {
        public:
            virtual bool isOpen() const = 0;
    };

    class FileReader
    {
        public:

    };

    class File
    {
        public:
            static std::vector<std::string> readLines(const std::string& filePath);
            static std::string readAsString(const std::string& filePath);
            static void writeLines(const std::string& filePath, const std::vector<std::string>& lines);
            static void writeString(const std::string& filePath, const std::string& string);
            
            //static void createDirectories(const std::string &path);

            File();
            File(const std::string& filePath);
            File(File&& __rhs) = default;
            ~File();

            File& operator=(File&& other) = default; // does not work with mutex

            bool isOpen() const;
            std::string asString();
            std::vector<std::string> getLines();
            std::string getLine(size_t n);
            
            void buildIndex();
        private:
            std::string m_filePath;
            std::vector<std::string> m_lines;
            std::ifstream m_file;
            std::mutex m_indexMutex;
            std::vector<std::streampos> m_offsets;
            size_t m_step;
    };

}


#endif /* BLUEMARBLE_FILE */
