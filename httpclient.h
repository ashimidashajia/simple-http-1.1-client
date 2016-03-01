#pragma once

#include "httpmultipartrequest.h"
#include "httpresponse.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/signals2/signal.hpp>

class http_request;

class http_client {
private:
    using response_raw_lines = std::vector<std::string>;
    
public:
    using success_handler = boost::function<void (http_response)>;
    using fail_handler = boost::function<void ()>;
    using progress_handler = boost::function<void(unsigned long, unsigned long)>;

    using success_sig = boost::signals2::signal<void (http_response)>;
    using fail_sig = boost::signals2::signal<void ()>;
    using progress_sig = boost::signals2::signal<void(unsigned long, unsigned long)>;
    
    http_client(boost::asio::io_service& io_service)
	: _socket(io_service),
	  _resolver(io_service)
    {}

    void get(const http_request& request);

    void post(http_multipart_request_ptr& request);

    void set_success_handler(success_handler handler);

    void set_fail_handler(fail_handler handler);

    void set_progerss_handler(progress_handler handler);

private:
    void http_worker(const http_request& request);

    void upload_worker(const http_multipart_request& request);

    void verify_response();
    
private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::ip::tcp::resolver _resolver;

    boost::thread _worker;
    
    success_handler _success_handler;
    fail_handler _fail_handler;
    progress_handler _progress_handler;

    success_sig _success_sig;
    fail_sig _fail_sig;
    progress_sig _progress_sig;

    http_multipart_request_ptr _mp_request;
};

using http_client_ptr = std::unique_ptr<http_client>;

inline
void http_client::set_success_handler(success_handler handler)
{
    _success_handler = handler;
    _success_sig.connect(_success_handler);
}

inline
void http_client::set_fail_handler(fail_handler handler)
{
    _fail_handler = handler;
    _fail_sig.connect(_fail_handler);
}

inline
void http_client::set_progerss_handler(progress_handler handler)
{
    _progress_handler = handler;
    _progress_sig.connect(_progress_handler);
}
