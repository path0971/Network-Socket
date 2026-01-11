#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

int main() {
	try {
		// io_context 객체 생성
		boost::asio::io_context io_context;

		// tcp 리졸버 생성
		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");

		// tcp 소켓 생성 및 서버에 연결
		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);

		// 사용자로부터 메시지 입력 받아 서버로 전송
		std::string message;
		while (std::getline(std::cin, message)) {
			message += "\n"; // 개행 문자 추가
			boost::asio::write(socket, boost::asio::buffer(message));
			
			// 서버로부터의 에코된 응답 읽기	
			boost::asio::streambuf buf;
			boost::asio::read_until(socket, buf, "\n");

			std::istream is(&buf);
			std::string reply;
			std::getline(is, reply);
			cout << "서버 응답: " << reply << endl;
		}
	} catch (std::exception& e) {
		cerr << "오류: " << e.what() << endl;
	}

	return 0;
}