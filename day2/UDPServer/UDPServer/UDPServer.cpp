#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace std;

int main() {
	try {
		//io_context 객체 생성
		boost::asio::io_context io_context;

		// UDP 소켓 생성 및 7777 포트바인딩
		udp::socket socket(io_context, udp::endpoint(udp::v4(), 7777));

		cout << "UDP Server 7777 대기중" << endl;

		while (true) {
			char data[1024];
			udp::endpoint sender_endpoint;
			boost::system::error_code error;

			// 클라이언트로부터 데이터 수신
			size_t length = socket.receive_from(boost::asio::buffer(data),
				sender_endpoint, 0, error);

			if (error && error != boost::asio::error::message_size) {
				throw boost::system::system_error(error);
			}

			cout << "수신한 메시지: " << std::string(data, length) << endl;

			// 수신한 데이터 클라이언트로 다시 전송 (에코)
			socket.send_to(boost::asio::buffer(data, length), sender_endpoint, 0, error);
		}
	}
	catch (std::exception& e) {
		cerr << "예외맨: " << e.what() << endl;
	}

	return 0;
}