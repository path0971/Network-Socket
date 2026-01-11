#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "AsyncEchoServer.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

/*
* io_context를 생성하고, Listener를 생성하여 실행하는 것이 목적으로,
* ioc.run()을 호출하여 io_context가 이벤트를 처리하도록 함
*/
int main() {
	try {
		// 서버 주소와 포트 설정
		auto const address = net::ip::make_address("0.0.0.0");
		auto const port = static_cast<unsigned short>(std::atoi("7777"));
		net::io_context ioc{ 1 };

		// 리스너 생성 및 실행
		std::make_shared<Listener>(ioc, tcp::endpoint{ address, port })->run();
		ioc.run();
	}
	catch (const std::exception& e) {
		std::cerr << "예외맨: " << e.what() << std::endl;
	}


	return 0;
}