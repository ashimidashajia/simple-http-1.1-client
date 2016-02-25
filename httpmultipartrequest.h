#pragma once

#include "httpurl.h"

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <ostream>
#include <iostream>

class http_multipart_request {
public:
    using data_array = std::vector<char>;
    using data_array_ptr = std::unique_ptr<data_array>;
    using parts = std::map<std::string, std::string>;

public:
    http_multipart_request(const http_url& url)
	: _url{url}
    {}

    http_multipart_request(const http_multipart_request& r)
	: _url{r._url}
    {
	std::cout << "copy c-tor\n";
    }
    
    http_multipart_request(http_multipart_request&& r)
	: _url{r._url} 
    {
	std::cout << "move c-tor\n";
    }

    void add_part(const std::string& name, const std::string& value);

    void add_file_part(const std::string& name, const std::string& filename,
		       const std::string& mimetype, data_array_ptr data);

    std::string host() const;

    data_array multipart_header() const;

    data_array multipart_body() const;

    const data_array& multipart_data() const;

    data_array::size_type multipart_data_size() const;

    std::string boundary() const;

    friend std::ostream& operator<<(std::ostream& os, const http_multipart_request& mpreq);

private:
    //    data_array header() const;

    //    data_array body() const;

    unsigned long content_size() const;

    std::string file_part() const;
    
private:
    http_url _url;
    parts _parts;
    std::string _name;    
    std::string _file_name;
    std::string _mimetype;
    data_array_ptr _pdata;
    mutable std::string _boundary;
};

using http_multipart_request_ptr = std::unique_ptr<http_multipart_request>;

inline
void http_multipart_request::add_part(const std::string& name, const std::string& value)
{
    _parts[name] = value;
}

inline
void http_multipart_request::add_file_part(const std::string& name, const std::string& filename,
					   const std::string& mimetype, data_array_ptr data)
{
    _name = name;
    _file_name = filename;
    _mimetype = mimetype;
    _pdata.swap(data);    
}

inline
const http_multipart_request::data_array&
http_multipart_request::multipart_data() const
{
    return *_pdata;
}

inline
http_multipart_request::data_array::size_type
http_multipart_request::multipart_data_size() const
{
    return _pdata->size();
}

inline
std::string http_multipart_request::host() const
{
    return _url.host();
}
