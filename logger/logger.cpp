#include "logger.h"
#include <chrono>
#include <iomanip>
#include <windows.h>

using namespace std;

namespace YellowWatcher{

    Logger* Logger::logger_ = nullptr;
    LoggerDestroyer Logger::destroyer_;
    
    LoggerDestroyer::~LoggerDestroyer() {   
        delete logger_; 
    }
    void LoggerDestroyer::initialize(Logger* logger) {
        logger_ = logger; 
    }

    Logger* Logger::getInstance() {
        if(!logger_)     {
            logger_ = new Logger();
            //logger_->file_ = nullptr;
            logger_->time_buffer_ = string(24, '\0');
            logger_->minimum_type_ = Type::Error;
            logger_->out_console_ = false;
            destroyer_.initialize(logger_);    
        }
        return logger_;
    }

    Logger::~Logger(){
        // if(file_){
        //     fclose(file_);
        // }
        if(fs_.is_open()){
            fs_.close();
        }
    }

    void Logger::Open(filesystem::path dir){
        // if(!file_){
        //     //file_ = _wfopen(dir.append(L"YellowWatcher.log").wstring().c_str(), L"ab");
        //     errno_t err = _wfopen_s(&file_, dir.append(L"YellowWatcher.log").wstring().c_str(), L"at+,CSS=UTF-8");
        //     wcout << dir.wstring() << L'\n';
        //     if(file_){
        //         wcout << L"File open\n";
        //     }
        //     else{
        //         wcout << L"File is not open, erorr=" << err << L'\n';
        //     }
        // }
        if(!fs_.is_open()){
            dir.append(L"YellowWatcher.log");
            fs_.open(dir, ios::out | ios::app | ios::binary);
        }
    }    

    wstring StringToWstring(const string& value){
        size_t size = mbstowcs(nullptr, value.c_str(), value.size());
        //wcstombs - wchar в char
        wstring wvalue(size, L'\000');
        mbstowcs(&wvalue[0], value.c_str(), value.size());

        return wvalue;
    }

    std::string WstringToUtf8(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    int sz = WideCharToMultiByte(65001, 0, &wstr[0], (int)wstr.size(), 0, 0, 0, 0);
    std::string res(sz, 0);
    WideCharToMultiByte(65001, 0, &wstr[0], (int)wstr.size(), &res[0], sz, 0, 0);
    return res;
}

    const string& Logger::CurTime(){
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t time = chrono::system_clock::to_time_t(now);
        const chrono::duration<double> tse = now.time_since_epoch();
        chrono::seconds::rep milliseconds = chrono::duration_cast<chrono::milliseconds>(tse).count() % 1000;
        ss_.str("");
        ss_ << setw(3) << setfill('0') << milliseconds;

        strftime(&time_buffer_[0], time_buffer_.size(), "%Y-%m-%d %H:%M:%S", localtime(&time));
        time_buffer_.replace(19, 1, ".");
        time_buffer_.replace(20, 3, ss_.str());
        time_buffer_.replace(23, 1, ";");

        return time_buffer_;        
    }

    void Logger::Print(const string& msg, Logger::Type type, bool anyway){

        Print(StringToWstring(msg), type, anyway);
    }

    void Logger::Print(const wstring& msg, Logger::Type type, bool anyway){

        if(!anyway && type < minimum_type_){
            return;
        }

        const string& cur_time = CurTime();
        fs_ << cur_time;
        if(out_console_) wcout << StringToWstring(cur_time);

        //type
        switch (type)
        {
        case Type::Trace:
            fs_ << u8"TRACE;";
            if(out_console_) wcout << L"TRACE;";
            break;
        case Type::Info:
            fs_ << u8"INFO;";
            if(out_console_) wcout << L"INFO;";
            break;
        case Type::Error:
            fs_ << u8"ERROR;";
            if(out_console_) wcout << L"ERROR;";
            break;
        default:
            break;
        }
        
        fs_ << WstringToUtf8(msg);
        if(out_console_) wcout << msg;
        
        fs_ << '\n';
        if(out_console_) wcout << L"\n";
        
        fs_.flush();
    }

    void Logger::Print(const string& msg, bool anyway){
        Print(msg, Type::Info, anyway);
    }

    void Logger::SetOutConsole(bool out_console){
        out_console_ = out_console;
    }

}