#pragma once

#include <string>
#include <vector>

class http_response {
public:
    using response_data = std::vector<char>;

    unsigned int _satus_code;    
    std::string _header;
    response_data _data;
};
