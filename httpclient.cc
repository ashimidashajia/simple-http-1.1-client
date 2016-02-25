/*
  clang++ -std=c++11 main.cc httpmanager.cc httpclient.cc httpurl.cc httprequest.cc httpmultipartrequest.cc -o client -lboost_system -lboost_thread -pthread
 */

#include "httpclient.h"
#include "httprequest.h"

#include <istream>
#include <ostream>
#include <string>
#include <sstream>
#include <chrono>

#include <boost/bind.hpp>

void http_client::get(const http_request& request)
{
    std::cout << "host to connect1: " << request.host() << "\n";
    
    boost::thread thr(boost::bind(&http_client::http_worker, this, request));
    boost::swap(_worker, thr);
    _worker.detach();
}

void http_client::http_worker(const http_request& request)
{
    using boost::asio::ip::tcp;

    tcp::resolver::query query{request.host(), "http"};

    tcp::resolver::iterator ep_it = _resolver.resolve(query);

    boost::asio::connect(_socket, ep_it);

    boost::asio::streambuf boost_request;
    std::ostream request_stream{&boost_request};
    request_stream << "GET " << request.str() << " HTTP/1.1\r\n"
		   << "Host: " << request.host() << "\r\n"
		   << "Accept */*\r\n"
		   << "onnection: keep-alive\r\n\r\n";

    boost::asio::write(_socket, boost_request);
    boost::asio::streambuf response;
    boost::asio::read_until(_socket, response, "\r\n");

    std::istream response_stream(&response);

    std::string http_version;
    unsigned int status_code;
    std::string status_message;

    response_stream >> http_version;
    response_stream >> status_code;

    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
	std::cerr << "Invalid response\n";
    }

    if (status_code != 200) {
	std::cerr << "Response returnes with status code "<< status_code << "\n";
    }

    std::cout << http_version << " " << status_code << " " << status_message << std::endl;
    
    boost::asio::read_until(_socket, response, "\r\n\r\n");

    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
	std::cout << header << "\n";
    }
    std::cout << "\n";

    std::stringstream ss;
    
    if (response.size() > 0) {
	//std::cout << "Response: " << &response;
	ss << &response;
    }

    boost::system::error_code error;
    while (boost::asio::read(_socket, response,
			     boost::asio::transfer_at_least(1), error)) {
	std::cout << "at least: " << &response;
    }

    if (error != boost::asio::error::eof) {
	std::cerr << "Error: " << error.message() << "\n";
	_fail_sig();
    } else {
	_success_sig(ss.str());
    }
}

void http_client::post(http_multipart_request_ptr& request)
{
    _mp_request.swap(request);
    
    boost::thread thr(boost::bind(&http_client::upload_worker, this, boost::cref(*_mp_request)));
    boost::swap(_worker, thr);
    
    _worker.detach();    
}

void http_client::upload_worker(const http_multipart_request& request)
{
    boost::system::error_code ec;
    
    using boost::asio::ip::tcp;

    tcp::resolver::query query{request.host(), "http"};

    tcp::resolver::iterator ep_it = _resolver.resolve(query, ec);

    if (ec.value() != boost::system::errc::success) {
	std::cerr << "resolver error: " << ec.message() << "\n";
	return;
    }
    
    boost::asio::connect(_socket, ep_it);

    http_multipart_request::data_array header_array =
	request.multipart_header();

    std::cout << "\n\n\tHeader start\n";    
    for (char c : header_array) {
	std::cout << c;
    }
    std::cout << "\tHeader end\n\n";
    
    boost::asio::write(_socket, boost::asio::buffer(header_array.data(),
						    header_array.size()));

    http_multipart_request::data_array body =
	request.multipart_body();


    std::cout << "\n\n\tBody start\n";        
    for (char c : body) {
	std::cout << c;
    }
    std::cout << "\tBody end\n\n";
    
    boost::asio::write(_socket, boost::asio::buffer(body.data(), body.size()));


    http_multipart_request::data_array::size_type bytes_sent{0};
    http_multipart_request::data_array::size_type bytes_left =
	request.multipart_data_size();

    unsigned packet_size = 1448;
    while (bytes_left) {
	if (packet_size > bytes_left) packet_size = bytes_left;
	http_multipart_request::data_array::size_type bytes_writes =
	    boost::asio::write(_socket, 
			       boost::asio::buffer(request.multipart_data().data() + bytes_sent,
						   packet_size));
	
	bytes_left -= bytes_writes;
	bytes_sent += bytes_writes;
        _success_sig(std::string{"sent "}.append(std::to_string(bytes_sent).append(" bytes")));
	boost::this_thread::sleep_for(boost::chrono::seconds(1));	
    }

    std::string bbuf = std::string{"\r\n"}.append(request.boundary());
    
    boost::asio::write(_socket, boost::asio::buffer(bbuf.data(), bbuf.size()));

    // Read data

    boost::asio::streambuf response;
    while (boost::asio::read(_socket, response,
			     /*boost::asio::transfer_at_least(1),*/ ec)) {
	std::cout << "answer: " << &response;
    }

    if (ec != boost::asio::error::eof) {
	std::cerr << "Error: " << ec.message() << "\n";
	_fail_sig();
    }
}




















