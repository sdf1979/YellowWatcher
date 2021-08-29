#pragma once

#include <string>
#include <filesystem>
#include "logger.h"

namespace YellowWatcher{

    class Settings{
        std::string host_;
        std::string http_host_;
        std::string http_port_;
        std::string http_target_;
        std::string http_login_;
        std::string http_password_;
        std::string path_monitoring_;

        std::string path_settings_;
    public:
        void CreateSettings(const std::filesystem::path& file_path);
        void Read(std::filesystem::path dir);
        std::string Host() const { return host_; }
        std::string HttpHost() const { return http_host_; }
        std::string HttpPort() const { return http_port_; }
        std::string HttpTarget() const { return http_target_; }
        std::string HttpLogin() const { return http_login_; }
        std::string HttpPassword() const { return http_password_; }
        std::string PathMonitoring() const { return path_monitoring_; }
        std::string PathSettings() const { return path_settings_; }
    };

}