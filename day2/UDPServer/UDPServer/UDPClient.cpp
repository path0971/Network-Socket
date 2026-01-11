#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace std;

int main() {
	try {
		// io_context 객체 생성
		boost::asio::io_context io_context;

		// UDP 소켓 생성
		udp::socket socket(io_context);

		// 서버의 엔드포인트 생성
		udp::endpoint server_endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 7777);

		// 사용자로부터 메시지 입력 받기
		std::string message;
		while (std::getline(cin, message)) {
			message += "\n";
			boost::system::error_code error;

			// 서버로 메시지 전송
			socket.send_to(boost::asio::buffer(message),
				server_endpoint, 0, error);

			char data[1024];
			udp::endpoint sender_endpoint;

			// 서버로부터 에코된 메시지 수신
			size_t length = socket.receive_from(boost::asio::buffer(data),
				sender_endpoint, 0, error);

			cout << "서버로부터 에코된 메시지: " << std::string(data, length) << endl;
		}
	}
	catch (exception& e) {
		cerr << "예외맨: " << e.what() << endl;
	}

	return 0;
}