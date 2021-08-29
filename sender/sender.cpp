#include "sender.h"
#include <iostream>

using namespace std;

namespace YellowWatcher{

    void fail(beast::error_code ec, char const* what){
        cerr << what << ": " << ec.message() << "\n";
    }

    class Session : public enable_shared_from_this<Session>{
        tcp::resolver resolver_;
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        http::request<http::string_body> req_;
        http::response<http::string_body> res_;
        std::string body; 
    public:
        explicit Session(net::io_context& ioc);
        void SetCounters(const std::string& counters);
        void Run(char const* host, char const* port, char const* target, std::string login, std::string password);
        void OnResolve(beast::error_code ec, tcp::resolver::results_type results);
        void OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
        void OnWrite(beast::error_code ec, std::size_t bytes_transferred);
        void OnRead(beast::error_code ec, std::size_t bytes_transferred);
    };

    Sender::Sender(std::string host, std::string port):
        host_(host),
        port_(port){}

    void Sender::Send(std::string target, std::string counters, std::string login, std::string password){
        net::io_context ioc;
        shared_ptr<Session> session = make_shared<Session>(ioc);
        session->SetCounters(counters);
        session->Run(&host_[0], &port_[0], &target[0], login, password);
        ioc.run();
    }

    Session::Session(net::io_context& ioc):
        resolver_(ioc),
        stream_(ioc){}

    void Session::SetCounters(const string& counters){
        body = u8"<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:c=\"http://1c.ru\">";
        body.append(u8"<soap:Header/>");
        body.append(u8"<soap:Body>");
        body.append(u8"<c:InputData>");
        body.append(u8"<c:inParam>");
        body.append(counters);
        body.append("</c:inParam>");
        body.append(u8"</c:InputData>");
        body.append(u8"</soap:Body>");
        body.append(u8"</soap:Envelope>");
    }

    void Session::Run(char const* host, char const* port, char const* target, std::string login, std::string password){
        string auth = login;
        auth.append(":");
        auth.append(password);
        
        string auth_base64;
        auth_base64.resize(base64::encoded_size(auth.size()));
        base64::encode(static_cast<void*>(&auth_base64[0]), static_cast<void*>(&auth[0]), auth.size());
        auth_base64 = "Basic " + auth_base64;

        req_.version(11);
        req_.method(http::verb::post);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.set(http::field::authorization, auth_base64);
        req_.set(http::field::accept_charset, "utf-8");
        req_.set(http::field::accept, "application/xml");
        req_.set(http::field::content_type, "application/xml;charset=utf-8");
        req_.body() = body;
        req_.prepare_payload();

        resolver_.async_resolve(host, port, beast::bind_front_handler(&Session::OnResolve, shared_from_this()));        
    }

    void Session::OnResolve(beast::error_code ec, tcp::resolver::results_type results){
        if(ec){
            return fail(ec, "resolve");
        }
        stream_.expires_after(std::chrono::seconds(5));
        stream_.async_connect(results, beast::bind_front_handler(&Session::OnConnect, shared_from_this()));
    }

    void Session::OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type){
        if(ec){
            return fail(ec, "connect");
        }

        stream_.expires_after(std::chrono::seconds(5));
        http::async_write(stream_, req_, beast::bind_front_handler(&Session::OnWrite, shared_from_this()));
    }

    void Session::OnWrite(beast::error_code ec, std::size_t bytes_transferred){
        boost::ignore_unused(bytes_transferred);

        if(ec){
            return fail(ec, "write");
        }
            
        http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&Session::OnRead, shared_from_this()));
    }

    void Session::OnRead(beast::error_code ec, std::size_t bytes_transferred){
        boost::ignore_unused(bytes_transferred);

        if(ec){
            return fail(ec, "read");
        }

        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        if(ec && ec != beast::errc::not_connected){
            return fail(ec, "shutdown");
        }
    }

}