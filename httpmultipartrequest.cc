#include "httpmultipartrequest.h"

#include <random>
#include <iterator>
#include <iostream>
#include <algorithm>

http_multipart_request::data_array
http_multipart_request::request() const
{
    http_multipart_request::data_array full_request;

    http_multipart_request::data_array request_body = multipart_body();

    const http_multipart_request::data_array data = multipart_data();

    request_body.insert(request_body.end(), data.begin(), data.end());

    const std::string bound = std::string{"\r\n"}.append(boundary());

    request_body.insert(request_body.end(), bound.begin(), bound.end());

    const http_multipart_request::data_array request_header =
	multipart_header(request_body.size());

    full_request.insert(full_request.end(), request_header.begin(), request_header.end());
    full_request.insert(full_request.end(), request_body.begin(), request_body.end());

    return full_request;
}

http_multipart_request::data_array
http_multipart_request::multipart_header(std::size_t bodysize) const
{
    std::string boundary_str = boundary();
    boundary_str = boundary_str.substr(2, boundary_str.length() - 2);
    
    std::string header_str = std::string{}
    .append("POST ").append(_url.uri()).append(" HTTP/1.1\r\n")
	 .append("Host: ").append(_url.host()).append(":")
	 .append(std::to_string(_url.port())).append("\r\n")
	 .append("Content-Type: multipart/form-data; ").append("boundary=")
	 .append(boundary_str)
	 .append("Accept-Encoding: gzip\r\n")
	 .append("MIME-Version: 1.0\r\n")	 
	 .append("Accept-Language: ru-US,en,*\r\n")
	 .append("User-Agent: Mozilla/5.0\r\n")
	 .append("Content-Length: ").append(std::to_string(bodysize)).append("\r\n")
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
