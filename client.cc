#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    try {
	if (argc != 3) {
	    std::cout << "Usage: " << argv[0] << " <server> [<path>]\n";
	    return EXIT_FAILURE;
	}

	boost::asio::io_service io_service;

	tcp::resolver resolver{io_service};
	tcp::resolver::query query{argv[1], "http"};
	tcp::resolver::iterator ep_it = resolver.resolve(query);

	tcp::socket socket{io_service};
	boost::asio::connect(socket, ep_it);

	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "GET " << argv[2]
		       << "?"
		       << "product=IRS_EASYFILESHARE10" << "&"
		       << "appversion=1.0.0.63" << "&"
		       << "device=Z10" << "&"
		       << "device_screensize=117.137" << "&"
		       << "device_screenwidth=768" << "&"
		       << "device_screenheight=1280" << "&"
		       << "device_has_keypad=0" << "&"
		       << "bbos=10.3.1.995" << "&"
		       << "bbos_hw=0" << "&"
		       << "bblang=ru" << "&"
		       << "platform=BLACKBERRY_10" << "&"
		       << "manufacturer=RIM%20BlackBerry" << "&"
		       << "ip_client=FE80:1::1" << "&"
		       << "connection=Unknown" << "&"
		       << "location_code=0" << "&"
		       << "country_code=" << "&"	    
		       << "simcard_networkcode=" << "&"
		       << "cellular_name=" << "&"
		       << "connection_CN=" << "&"
		       << "connection_NT=" << "&"
		       << "connection_SL=" << "&"
		       << "connection_SB=" << "&"
		       << " HTTP/1.1\r\n";
	request_stream << "Host: " << argv[1] << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: keep-alive\r\n\r\n";

	boost::asio::write(socket, request);

	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n");
	
	std::istream response_stream(&response);
	
	
	std::string http_version;
	response_stream >> http_version;
	
	unsigned int status_code;
	response_stream >> status_code;

	std::string status_message;
	std::getline(response_stream, status_message);

	if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
	    std::cout << "Invalid response\n";
	}

	if (status_code != 200) {
	    std::cout << "Response returnes with status code "
		      << status_code << "\n";
	}

	std::cout << http_version << " " << status_code << " " << status_message << std::endl;
	
	boost::asio::read_until(socket, response, "\r\n\r\n");

	std::string header;
	while (std::getline(response_stream, header) && header != "\r") {
	    std::cout << header << "\n";
	}

	std::cout << "\n";

	if (response.size() > 0) {
	    std::cout << &response;
	}

	boost::system::error_code error;
	while (boost::asio::read(socket, response,
				 boost::asio::transfer_at_least(1), error)) {
	    std::cout << &response;
	}

	if (error != boost::asio::error::eof) {
	    throw boost::system::system_error(error);
	}
    } catch (const std::exception& e) {
	std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
