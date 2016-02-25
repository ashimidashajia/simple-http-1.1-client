#include "httprequest.h"

#include <algorithm>
#include <iterator>

std::string http_request::str() const
{
    std::string request{_url.uri()};
    request.append("?");
    
    for (const std::pair<const std::string, std::string>& pair : _params) {
	request.append(pair.first).append("=").append(pair.second).append("&");
    }

    request = request.substr(0, request.length() - 1);

    std::replace(std::begin(request), std::end(request), ' ', '+');

    return request;
}
