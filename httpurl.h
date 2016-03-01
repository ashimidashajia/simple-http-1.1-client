#pragma once

#include <string>

class http_url {
public:
    enum class error_types {
	Success,
	    EmptyUrl,
	    NoHostnameFound,
	    NoUriFound
    };

public:
    http_url(const std::string& url);

    std::string host() const;

    std::string uri() const;

    unsigned short port() const;

    error_types error() const;


private:
    std::string _host;
    unsigned short _port;    
    std::string _uri;

    error_types _error;
};

inline
std::string http_url::host() const
{
    return _host;
}

inline
std::string http_url::uri() const
{
    return _uri;
}

inline
unsigned short http_url::port() const
{
    return _port;
}

inline
http_url::error_types http_url::error() const
{
    return _error;
}
