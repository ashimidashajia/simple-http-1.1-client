#pragma once

#include "httpmultipartrequest.h"

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/signals2/signal.hpp>

class http_request;

class http_client {
public:
    using SuccessHandler = boost::function<void (const std::string&)>;
    using FailHandler = boost::function<void ()>;

    using SuccessSig = boost::signals2::signal<void (const std::string&)>;
    using FailSig = boost::signals2::signal<void ()>;
    
    http_client(boost::asio::io_service& io_service)
	: _socket(io_service),
	  _resolver(io_service)
    {}

    void get(const http_request& request);

    void post(http_multipart_request_ptr& request);

    void set_success_handler(SuccessHandler handler);

    void set_fail_handler(FailHandler handler);

private:
    void http_worker(const http_request& request);

    void upload_worker(const http_multipart_request& request);
    
private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;

    boost::thread _worker;
    
    SuccessHandler _success_handler;
    FailHandler _fail_handler;

    SuccessSig _success_sig;
    FailSig _fail_sig;

    http_multipart_request_ptr _mp_request;
};

using http_client_ptr = std::unique_ptr<http_client>;

inline
void http_client::set_success_handler(SuccessHandler handler)
{
    _success_handler = handler;
    _success_sig.connect(_success_handler);
}

inline
void http_client::set_fail_handler(FailHandler handler)
{
    _fail_handler = handler;
    _fail_sig.connect(_fail_handler);
}
