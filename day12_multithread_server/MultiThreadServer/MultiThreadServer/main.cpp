#include "AsyncMultiThreadServer.h"

int main() {
	try {
		auto const address = net::ip::make_address("127.0.0.1");
		auto const port = static_cast<unsigned short> (std::atoi("7777"));
		auto const threads = std::max<int>(1, std::thread::hardware_concurrency());

		net::io_context ioc{ threads }; // thread pool로 io_context 생성

		// 서버 리스너 시작
		std::make_shared<Listener>(ioc, tcp::endpoint{ address, port })->run();

		std::vector<std::thread> v; // 스레드 풀 생성
		v.reserve(threads - 1); // 메인 스레드 제외
		for (auto i = threads - 1; i > 0; --i) { // workers 시작
			v.emplace_back([&ioc] { // 스레드 정의
				ioc.run();
				});
		}

		ioc.run(); // 메인 스레드도 io_context 실행
	}
	catch (std::exception& e) {
		std::cerr << "예외맨: " << e.what() << std::endl;
	}

	return 0;
}