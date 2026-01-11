#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "AsyncRestServer.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
	try {
		auto const address = net::ip::make_address("0.0.0.0");
		auto const port = static_cast<unsigned short>(std::atoi("7777"));
		net::io_context ioc{ 1 };

		std::make_shared<AsyncRestServer>(ioc, tcp::endpoint{ address,port })->run();
		ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << "¿¹¿Ü¸Ç: " << e.what() << std::endl;
	}

	return EXIT_SUCCESS;


	return 0;
}