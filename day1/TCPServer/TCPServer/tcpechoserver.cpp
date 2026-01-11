#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

int main() {
	try {
		// io_context 객체 생성
		boost::asio::io_context io_context;

		// tcp 수신자 생성, 포트 12345에서 수신 대기
		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

		cout << "서버는 포트 12345에서 수신 대기중!" << endl;

		while (true) {
			// 새로운 소켓 생성
			tcp::socket socket(io_context);

			// 클라이언트 연결 수락
			acceptor.accept(socket);

			// 클라이언트로부터 데이터 수신
			boost::asio::streambuf buffer;
			boost::asio::read_until(socket, buffer, "\n"); //개행까지

			std::istream is(&buffer);
			std::string message;
			std::getline(is, message);
			cout << "수신한 메시지: " << message << endl;

			// 클라이언트에게 받은 데이터 고대로 다시 전송(에코)
			boost::asio::write(socket, boost::asio::buffer(message + "\n"));
		}
	}
	catch (std::exception& e) {
			cerr << "오류: " << e.what() << endl;
		}
		
	return 0;
}