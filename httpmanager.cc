#include "httpmanager.h"

#include "boost/move/move.hpp"

http_client_ptr http_manager::make_http_client_ptr() const
{
    return boost::move(http_client_ptr{new http_client{_io_service}});
}

boost::asio::io_service http_manager::_io_service;
