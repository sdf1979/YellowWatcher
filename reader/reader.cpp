#include "reader.h"
#include <filesystem>
#include <cstring>

//TODO для отладки
#include <iostream>

namespace YellowWatcher{

    using namespace std;

    time_t GetFileTime(const string& file_name){
        filesystem::path path = file_name;
        string file = path.stem().string();
        std::tm tm = {};
        tm.tm_year = stoi("20" + file.substr(0, 2)) - 1900;
        tm.tm_mon = stoi(file.substr(2, 2)) - 1;
        tm.tm_mday = stoi(file.substr(4, 2));
        tm.tm_hour = stoi(file.substr(6, 2));
        return std::mktime(&tm);
    }

    int FindStartPositionBuffer(char* buffer, uint64_t size){
        char* ch = buffer + size - 1;
        int pos = 0;
        for(;;){
            if(pos == 0 && *ch == '-'){
                ++pos;
            }
            else if(pos >=1 && pos <= 6){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 7){
                if(*ch == '.') ++pos; else pos = 0;
            }
            else if(pos >=8 && pos <= 9){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 10){
                if(*ch == ':') ++pos; else pos = 0;
            }
            else if(pos >=11 && pos <= 12){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 13){
                if(*ch == '\n' || *ch == -65){
                    return (buffer + size - 1- ch)/sizeof(char);
                }
            }
            else{
                pos = 0;
            }

            if(ch == buffer){
                return -1;
            }

            --ch;
        }

        return -1;
    }

    Reader::Reader(string file_name, unsigned int size_buffer):
        pos_(0),
        buffer_(nullptr),
        size_read_(0),
        file_name_(file_name),
        size_buffer_(size_buffer),
        file_time_(GetFileTime(file_name)),
        MAX_FILE__PROCESSING_TIME(3610)
        {}

    void Reader::FindStartPosition(){
        fs_.seekg (0, ios::end);
        pos_end_ = fs_.tellg();
        uint64_t size = 0;
        for(;;){
            if(pos_end_ > size_buffer_) {
                pos_ = pos_end_ - size_buffer_;
            }
            else {
                pos_ = 0;
            }

            size = pos_end_ - pos_;            
            fs_.seekg( pos_, ios::beg);
            fs_.read(buffer_, size);
            
            int end_offset = FindStartPositionBuffer(buffer_, size);
            if(end_offset != -1){
                pos_ = pos_end_ - end_offset;
                break;
            }

            pos_end_ -= size;

            if(pos_ == 0){
                break;
            }
        }
    }

    uint64_t Reader::GetFileSize(){
        std::ifstream file_stream(file_name_, ios::binary);
        const auto begin = file_stream.tellg();
        file_stream.seekg (0, ios::end);
        const auto end = file_stream.tellg();
        file_stream.close();
        return end - begin;
    }

    bool Reader::Open(){
        if (!fs_.is_open()){
            fs_.open(file_name_, ios::binary);
            if(fs_.is_open()){
                buffer_ = new char[size_buffer_];
                FindStartPosition();
            }
        }
        return fs_.is_open();
    }

    bool Reader::Read(){
        fs_.seekg (0, ios::end);
        pos_end_ = fs_.tellg();

        //cout << "File: " << file_name_ << "; Size: " << pos_end_ << "; Read: " << (pos_end_ - pos_) << '\n';

        return pos_ != pos_end_;      
    }

    bool Reader::Next(){
        size_read_ = pos_end_ - pos_;
        size_read_ = (size_read_ > size_buffer_) ? size_buffer_ : size_read_;
        // if(size_read_){
        //     cout << "Next: " << size_read_ << '\n';
        // }

        fs_.seekg( pos_, ios::beg);
        fs_.read(buffer_, size_read_);
        pos_ += size_read_;

        return size_read_;
    }

    bool Reader::IsWorkingFile(){
        return IsWorkingFile(chrono::system_clock::to_time_t(chrono::system_clock::now()));
    }

    bool Reader::IsWorkingFile(time_t cur_time){
        uint64_t size = GetFileSize();
        if(size <= 3){
            return false;
        }
        return cur_time - file_time_ <= MAX_FILE__PROCESSING_TIME;
    }    

    const string& Reader::GetFileName(){
        return file_name_;
    }

    pair<const char*, int> Reader::GetBuffer(){
        return {buffer_, size_read_};
    }

    void Reader::ClearBuffer(){
        memset(buffer_, '\0', size_buffer_);
    }

    Reader::~Reader(){
        //cout << "Destructor: " << file_name_ << '\n';
        if(fs_.is_open()){
            fs_.close();
        }
        if(buffer_){
            delete[] buffer_;
        }
    }

    bool Reader::operator==(const Reader& reader) const{
        return this == &reader;
    }
}