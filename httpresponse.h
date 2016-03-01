#pragma once

#include <string>
#include <memory>
#include <vector>

class http_response {
public:
    using response_raw_lines = std::vector<std::string>;
    
    using response_data = std::string;
    using response_data_ptr = std::shared_ptr<response_data>;

    struct header_info {
	std::string http_version;
	unsigned int status_code;
	std::string status_message;
    };
    
public:
    http_response(const response_raw_lines& lines);

    const header_info& header() const;

    const response_data& data() const;

private:
    void parse_lines(const response_raw_lines& lines);
    
private:
    header_info _header_info;
    response_data_ptr _pdata;
};

inline
const http_response::header_info& http_response::header() const
{
    return _header_info;
}

inline
const http_response::response_data& http_response::data() const
{
    return *_pdata;
}
