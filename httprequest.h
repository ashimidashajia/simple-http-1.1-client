#pragma once

#include "httpurl.h"

#include <map>

class http_request {
public:
    using params = std::map<std::string, std::string>;

    http_request(const http_url& url)
	: _url{url}
    {}

    void set_url(const http_url& url);
    
    void add_params(const std::string& name, const std::string& value);

    std::string host() const;

    std::string str() const;

private:
    params _params;
    http_url _url;
};

inline
void http_request::set_url(const http_url& url)
{
    _url = url;
}

inline
void http_request::add_params(const std::string& name, const std::string& value)
{
    _params[name] = value;
}

inline
std::string http_request::host() const
{
    return _url.host();
}
