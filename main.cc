#include "httpmanager.h"
#include "httpclient.h"
#include "httpurl.h"
#include "httprequest.h"
#include "httpmultipartrequest.h"
#include "httpresponse.h"

#include <boost/thread/mutex.hpp>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <utility>

class Printer {
public:
    void operator()(const std::string& str)
    {
	boost::lock_guard<boost::mutex> guard(_mutex);
	fprintf(stdout, "%s\n", str.c_str());
    }

private:
    boost::mutex _mutex;
};

class App {
public:
    App()
	: _is_done{false}
    {
	http_client_ptr ptr = http_manager{}.make_http_client_ptr();
	_phttp_client.swap(ptr);

	_phttp_client->set_success_handler(boost::bind(boost::mem_fn(&App::success_handler), this, _1));
	_phttp_client->set_fail_handler(boost::bind(&App::fail_handler, this));
	_phttp_client->set_progerss_handler(boost::bind(boost::mem_fn(&App::progress_handler), this, _1, _2));	
    }

    bool foo(http_multipart_request::data_array&& file)
    {
	http_url url{"http://irsoftware.ru:80/easyfileshare/bin/upload.php"};
	if (url.error() != http_url::error_types::Success) {
	    std::cerr << "Bad url\n";
	    return false;
	}
	
	http_multipart_request_ptr mp_request{new http_multipart_request{url}};
	mp_request->add_part("product", "IRS_EASYFILESHARE10");
	mp_request->add_part("appversion","1.0.0.64");
	mp_request->add_part("license_id","1");
	mp_request->add_part("license_email","");
	mp_request->add_part("license_type","DEMO");
	mp_request->add_part("bbpin","4294902937");
	mp_request->add_part("bbmodel","All Touch [1280x768]");
	mp_request->add_part("bbos","10.3.1.995");
	mp_request->add_part("bblang","ru");
	mp_request->add_part("platform","BLACKBERRY_10");
	mp_request->add_part("manufactured","RIM BlackBerry");
	mp_request->add_part("connection","Unknown");
	mp_request->add_part("country_code","");
	mp_request->add_part("cellular_name","");
	mp_request->add_part("simcard_networkcode","");
	mp_request->add_part("connection_CN","");
	mp_request->add_part("connection_SB","");
	mp_request->add_part("connection_SL","");
	mp_request->add_part("connection_NT","");
	mp_request->add_part("orig_filename","file://test.jpg");
	mp_request->add_part("orig_file_size","117166");
	mp_request->add_part("orig_width","1280");
	mp_request->add_part("orig_height","1024");
	mp_request->add_part("orig_orientation","0");
	mp_request->add_part("orig_type","image/jpeg");
	mp_request->add_part("resized_file_size","0");
	mp_request->add_part("resized_width","0");
	mp_request->add_part("resized_height","0");
	mp_request->add_part("password","");
	mp_request->add_part("note","");
	mp_request->add_part("user_file_name","");
	mp_request->add_part("marked_as_deleted","0");
	mp_request->add_part("reason_marked_as_deleted","1");
	mp_request->add_part("time_needed_client","10");
	mp_request->add_part("location_code","0");
	mp_request->add_part("technology","None");
	mp_request->add_part("connection_SL","0");
	mp_request->add_part("connection_SB","0");

	mp_request->add_file_part("fileToUpload", "test.jpg", "image/jpeg",
				  http_multipart_request::data_array_ptr{new http_multipart_request::data_array{file}});
	
	_phttp_client->post(mp_request);
	
	return true;
    }

    bool is_done() const { return _is_done; }

private:
    void success_handler(http_response response)
    {
	std::cout << "Protocol version: " << response.header().http_version
		  << "\nStatus code: " << response.header().status_code
		  << "\nStatus message: " << response.header().status_message
		  << "\nData: " << response.data()
		  << "\n";
	_is_done = true;
    }

    void fail_handler()
    {
	std::cout << "request failed\n";
    }

    void progress_handler(unsigned long sent, unsigned long total)
    {
	std::cout << "Sent " << sent << " bytes from " << total << std::endl;
    }
    
private:
    http_client_ptr _phttp_client;
    
    bool _is_done;    
};

int main(int argc, char* argv[])
{

    std::ifstream ifs{"test.jpg", std::ios_base::ate};
    if (!ifs.is_open()) {
	std::cerr << "Failed to open file 'test.jpg'\n";
	return EXIT_FAILURE;
    }

    const std::ifstream::pos_type file_size = ifs.tellg();
    
    std::cout << "File size: " << file_size << " bytes\n";

    ifs.seekg(0);

    http_multipart_request::data_array file_data(file_size);

    unsigned int block_size = 4096;
    
    unsigned int bytes_left = file_size;
    unsigned int bytes_read{0};

    for (std::vector<char> a(block_size); bytes_left && ifs;) {
	if (block_size > bytes_left) block_size = bytes_left;		
	ifs.read(a.data(), block_size);
	std::memcpy(file_data.data() + bytes_read, a.data(), block_size);
	bytes_left -= block_size;
	bytes_read += block_size;
    }

    App app;

    for (int i = 0; i < 10000000; ++i) {
	if (i == 2) {
	    if (!app.foo(std::move(file_data))) break;
	}
	
	boost::this_thread::sleep_for(boost::chrono::seconds(1));
	//std::cout << "main-id " << boost::this_thread::get_id() << "\n";	
    }

    return EXIT_SUCCESS;
}










