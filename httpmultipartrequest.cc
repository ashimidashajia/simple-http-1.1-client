#include "httpmultipartrequest.h"

#include <random>
#include <iterator>
#include <iostream>

http_multipart_request::data_array
http_multipart_request::multipart_header() const
{
    std::string header_str = std::string{}
    .append("POST ").append(_url.uri()).append(" HTTP/1.1\r\n")
	 .append("Host: ").append(_url.host()).append(":")
	 .append(std::to_string(_url.port())).append("\r\n")
	 .append("Content-Type: multipart/form-data; ").append("boundary=")
	 .append("MIME-Version: 1.0\r\n")
	 .append(boundary()).append("Accept-Encoding: gzip\r\n")
	 .append("Accept-Language: ru-US,en,*\r\n")
	 .append("User-Agent: Mozilla/5.0\r\n")
	 //.append("Content-Length: ").append(std::to_string(content_size())).append("\r\n")
	 .append("Connection: Keep-Alive\r\n").append("\r\n");
    
    return data_array{header_str.begin(), header_str.end()};
}

http_multipart_request::data_array
http_multipart_request::multipart_body() const
{
    data_array data{};

    std::string boundary_str = boundary();
    
    for (const std::pair<const std::string, std::string>& pair : _parts) {
	std::string part{boundary_str};
	part.append("Content-Disposition: form-data; name=\"")
	    .append(pair.first).append("\"\r\n\r\n")
	    .append(pair.second).append("\r\n");
	std::copy(part.begin(), part.end(), std::back_inserter(data));	
    }

    std::string file_part_str =
	std::string{boundary_str}.append(file_part()).append("\r\n");//.append(boundary_str);
    std::copy(file_part_str.begin(), file_part_str.end(),
	      std::back_inserter(data));

    return data;
}

std::string http_multipart_request::boundary() const
{
    if (!_boundary.length()) {
	constexpr unsigned int max_length = 30;

	std::mt19937 mt_gen{};
	std::uniform_int_distribution<unsigned char> dis(65, 122);

	std::size_t index{0};
	_boundary.append("---------------");

	while (index < max_length) {
	    mt_gen.seed(std::random_device{}());
	    unsigned char num = dis(mt_gen);
	    if (num < 91 || num > 96) {
		_boundary.append(1, num);
		++index;
	    }
	}

	_boundary.append("\r\n");
    }
    
    return _boundary;
}

unsigned long http_multipart_request::content_size() const
{
    unsigned long size{0};

    for (const std::pair<const std::string, std::string>& pair : _parts) {
	size += pair.second.length();
    }

    size += _pdata->size();

    return size;
}

std::ostream& operator<<(std::ostream& os, const http_multipart_request& mpreq)
{
    http_multipart_request::data_array data = mpreq.multipart_body();
    
    for (http_multipart_request::data_array::value_type value : data) {
	os << value;
    }

    return os;
}

std::string http_multipart_request::file_part() const
{
    std::string part{"Content-Disposition: form-data; "};
    part.append("name=\"").append(_name).append("\"; ")
	.append("filename=\"").append(_file_name).append("\"\r\n")
	.append("Content-Type: ").append(_mimetype).append("\r\n");
    return part;
}
