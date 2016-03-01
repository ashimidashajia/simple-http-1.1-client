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
#include <algorithm>
#include <iterator>
#include <cstdio>

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

    boost::system::error_code err;
    
    boost::asio::streambuf response;
    boost::asio::read(_socket, response, err);
    std::istream response_stream(&response);    

    response_raw_lines lines{};

    std::string line{};
    while (std::getline(response_stream, line)) {
	lines.push_back(line.substr(0, line.length() - 1));
    }

    bool is_http_response{false};

    std::for_each(lines.begin(), lines.end(), [&is_http_response] (const std::string& str) {
	    if (str.find("HTTP/") != std::string::npos) is_http_response = true;
	});

    if (!is_http_response) _fail_sig();

    http_response httpresp{lines};
    
    if (err != boost::asio::error::eof) {
	std::cerr << "Error: " << err.message() << "\n";
	_fail_sig();
    }
    
    /*
    
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
    */
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

    const http_multipart_request::data_array full_request = request.request();

    http_multipart_request::data_array::size_type bytes_sent{0};
    http_multipart_request::data_array::size_type bytes_left = full_request.size();
    http_multipart_request::data_array::size_type bytes_total = bytes_left;

    unsigned packet_size = 1440;

    _progress_sig(0, bytes_total);
    while (bytes_left) {
	boost::asio::socket_base::bytes_readable command(true);
	_socket.io_control(command);
	if (command.get()) {
	    std::cout << "\n\nHave unreaded bytes in socket!\n";
	    boost::asio::streambuf response;
	    while (boost::asio::read(_socket, response, ec)) {
		std::cout << "answer: " << &response;
	    }
	    std::cout << "\n\n";
	}
	
	if (packet_size > bytes_left) packet_size = bytes_left;
	http_multipart_request::data_array::size_type bytes_writes =
	    boost::asio::write(_socket, 
			       boost::asio::buffer(full_request.data() + bytes_sent,
						   packet_size));
	
	bytes_left -= bytes_writes;
	bytes_sent += bytes_writes;
	_progress_sig(bytes_sent, bytes_total);
    }

    // Read data

    boost::system::error_code err;
    
    boost::asio::streambuf response;
    boost::asio::read(_socket, response, err);
    std::istream response_stream(&response);    

    response_raw_lines lines{};

    std::string line{};
    while (std::getline(response_stream, line)) {
	lines.push_back(line.substr(0, line.length() - 1));
    }

    bool is_http_response{false};

    std::for_each(lines.begin(), lines.end(), [&is_http_response] (const std::string& str) {
	    if (str.find("HTTP/") != std::string::npos) is_http_response = true;
	});

    if (!is_http_response) _fail_sig();

    _success_sig(http_response{lines});
    
    if (err != boost::asio::error::eof) {
	std::cerr << "Error: " << err.message() << "\n";
	_fail_sig();
    }
}




















