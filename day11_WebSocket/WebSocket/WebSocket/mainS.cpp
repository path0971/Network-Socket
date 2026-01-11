#include "AsyncWebSocketServer.h"

int main() {
	try {
		// 서버 주소 및 포트 설정
		auto const address = net::ip::make_address("0.0.0.0");
		auto const port = static_cast<unsigned short> (std::atoi("7777"));
		net::io_context ioc{ 1 };

		// 리스너 생성 및 실행
		std::make_shared<listener>(ioc, tcp::endpoint{ address, port })->run();
		ioc.run();
	}
	catch (std::exception const& e) {
		std::cerr << "예외맨: " << e.what() << std::endl;
	}

	return EXIT_SUCCESS;


	return 0;
}