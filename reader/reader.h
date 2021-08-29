#pragma once

#include <fstream>
#include <cstdint>
#include <string>
#include <time.h>

namespace YellowWatcher{

    class Reader{
        std::ifstream fs_;
        std::uint64_t pos_;
        std::uint64_t pos_end_;
        char* buffer_;
        int size_read_;
        std::string file_name_;
        int size_buffer_;
        time_t file_time_;
        const time_t MAX_FILE__PROCESSING_TIME;
        void FindStartPosition();
        uint64_t GetFileSize();
    public:
        Reader(std::string file_name, unsigned int size_buffer);
        bool Open();
        bool Read();
        bool Next();
        bool IsWorkingFile();
        bool IsWorkingFile(time_t cur_time);
        const std::string& GetFileName();
        std::pair<const char*, int> GetBuffer();
        void ClearBuffer();
        ~Reader();
        bool operator==(const Reader& reader) const;
    };

}