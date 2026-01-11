#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
	try {
		boost::asio::io_context io_context;

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");
		// “127.0.0.1:12345”를 엔드포인트로 변환!

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints); // 서버에 연결!

		while(true) {
			boost::asio::streambuf buf;
			boost::asio::read_until(socket, buf, "\n"); // 동기적으로 서버로부터 메시지 수신!

			std::istream is(&buf); // 수신된 데이터를 스트림으로 변환!
			std::string message;
			std::getline(is, message);
			std::cout << "Received: " << message << std::endl;
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}