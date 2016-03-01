#include "httpresponse.h"

#include <cstdio>
#include <sstream>
#include <iostream>

http_response::http_response(const http_response::response_raw_lines& lines)
    : _pdata{new response_data{}}
{
    parse_lines(lines);
}

void http_response::parse_lines(const http_response::response_raw_lines& lines)
{
    bool header_found{false};
    bool data_found{false};
    
    response_raw_lines::const_iterator it = lines.begin();
    response_raw_lines::const_iterator end_it = lines.end();
    
    for (; it != end_it; ++it) {
	if ((!header_found) && (it->find("HTTP/") != std::string::npos)) {
	    std::stringstream ss{*it};
	    
	    ss >> _header_info.http_version;
	    ss >> _header_info.status_code;
	    std::getline(ss, _header_info.status_message);

	    header_found = true;
	}

	if ((!data_found) && ((*it).length() == 0)) {
	    response_raw_lines::const_iterator data_it = it + 1;
	    
	    for (; data_it != end_it; ++ data_it) {
		_pdata->insert(_pdata->end(), (*data_it).begin(), (*data_it).end());
	    }

	    data_found = true;
	}
    }
}
