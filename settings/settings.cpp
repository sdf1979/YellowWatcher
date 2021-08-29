#include "settings.h"
#include <fstream>

using namespace std;

namespace fs = std::filesystem;

namespace YellowWatcher{

    static auto LOGGER = YellowWatcher::Logger::getInstance();
    
    void Settings::CreateSettings(const fs::path& file_path){
        if(!fs::exists(file_path)){
            ofstream out(file_path);
            out
            << "#host=localhost\n"
            << "#http_host=host for sending counters\n"
            << "#http_port=80\n"
            << "#http_target=/QMC/ws/InputStatistics.1cws\n"
            << "#http_login=Incident\n"
            << "#http_password=Incident\n"
            << "#path_monitoring=C:\\LOGS"
            ; 
            out.close();
            LOGGER->Print(string("Create file settings: ").append(file_path.string()), true);
        }
    }

    pair<string, string> ParseLine(const string& line){
        string_view line_sv(line);
        auto pos = line_sv.find("=");
        if(pos != wstring_view::npos){
            string key(line_sv.substr(0, pos));
            line_sv.remove_prefix(++pos);
            return make_pair(key, string(line_sv));
        }
        return {};
    }

    void Settings::Read(fs::path dir){
        fs::path file_path = dir.append("settings.txt");
        path_settings_ = file_path.string();
        if(!fs::exists(file_path)){
            CreateSettings(file_path);
        }
        ifstream in(file_path);
        if (in.is_open()) {
            string line;
            while (getline(in, line)) {
                pair<string, string> key_value = ParseLine(line);
                if(key_value.first == "host"){
                    host_ = key_value.second;
                }
                else if(key_value.first == "http_host"){
                    http_host_ = key_value.second;
                }
                else if(key_value.first == "http_port"){
                    http_port_ = key_value.second;
                }
                else if(key_value.first == "http_target"){
                    http_target_ = key_value.second;
                }
                else if(key_value.first == "http_login"){
                    http_login_ = key_value.second;
                }
                else if(key_value.first == "http_password"){
                    http_password_ = key_value.second;
                }
                else if(key_value.first == "path_monitoring"){
                    path_monitoring_ = key_value.second;
                }
            }
            in.close();
        }
    }
}