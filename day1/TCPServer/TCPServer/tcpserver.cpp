#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
	try {
		boost::asio::io_context io_context; // IO 컨텍스트 생성

		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345)); // 포트 12345에서 수신 대기

		std::cout << "Server is listening on port 12345!" << std::endl;

		while (true) {
			tcp::socket socket(io_context); // 소켓 생성
			acceptor.accept(socket); // 클라이언트 연결 수락

			std::string message = "HiHi from Server!"; // 서버에서 전송할 메시지
			boost::system::error_code ignored_error; // 오류 코드 무시
			boost::asio::write(socket, boost::asio::buffer(message), ignored_error); // 메시지 전송
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}