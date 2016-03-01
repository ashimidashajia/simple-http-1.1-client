#include "httpurl.h"

#include <iostream>

http_url::http_url(const std::string& url)
    : _error{error_types::Success}
{
    if (!url.length()) {
	_error = error_types::EmptyUrl;
	return;
    }

    std::string url_copy = url;

    if (url.find("http://") != std::string::npos) {
	url_copy = url.substr(7, url.length() - 7);
    }

    std::string::size_type port_pos = url_copy.find(":");
    std::string::size_type host_end = url_copy.find("/");
    
    if (port_pos != std::string::npos && host_end != std::string::npos) {
	_host = url_copy.substr(0, port_pos);
	_port = std::stoi(url_copy.substr(port_pos + 1, host_end - port_pos - 1));
	_uri = url_copy.substr(host_end, url_copy.length() - host_end);    	
    } else if (host_end != std::string::npos) {
	_host = url_copy.substr(0, host_end);
	_uri = url_copy.substr(host_end, url_copy.length() - host_end);    		
    }

    if (!_host.length()) {
	_error = error_types::NoHostnameFound;
	return;
    }

    if (!_uri.length()) {
	_error = error_types::NoUriFound;
	return;
    }
}
