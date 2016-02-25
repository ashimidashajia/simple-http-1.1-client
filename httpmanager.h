#pragma once

#include "httpclient.h"
#include "httprequest.h"

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

class http_manager : private boost::noncopyable {
public:
    http_client_ptr make_http_client_ptr() const;
    
private:
    static boost::asio::io_service _io_service;
};
