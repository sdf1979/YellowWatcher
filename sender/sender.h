#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/detail/base64.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace base64 = beast::detail::base64;

using tcp = net::ip::tcp;

namespace YellowWatcher{

    class Sender{
        std::string host_;
        std::string port_;
    public:
        Sender(std::string host, std::string port);
        void Send(std::string target, std::string counters, std::string login, std::string password);
    };

}