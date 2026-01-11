#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

class TcpClient {
	tcp::resolver resolver_;
	tcp::socket socket_;
	char data_[1024];

public:
	TcpClient(boost::asio::io_context& io_context,
		const std::string& host, const std::string& service) : resolver_(io_context), socket_(io_context) {
		connect(host, service);
	}
	~TcpClient() { // 혹시 소켓이 살아있을 경우를 대비하는 소멸자임ㅇㅇ
		if (socket_.is_open()) {
			boost::system::error_code ec;
			socket_.close(ec);
		}
	}

private:
	void connect(const std::string& host, const std::string& service) {
		auto endpoints = resolver_.resolve(host, service);
		boost::asio::async_connect(socket_, endpoints,
			[this](const boost::system::error_code& error, const tcp::endpoint&) {
				if (!error) {
					do_write();
				}
			});
	}

	void do_write() {
		std::string message = "HiHi from clients";
		boost::asio::async_write(socket_, boost::asio::buffer(message),
			[this](const boost::system::error_code& error, std::size_t) {
				if (!error) {
					do_read();
				}
			}
		);
	}

	void do_read() {
		socket_.async_read_some(boost::asio::buffer(data_),
			[this](const boost::system::error_code& error, std::size_t length) {
				if (!error) {
					cout << "서버로부터 수신한 메시지: " << string(data_, length) << endl;
				}
			}
		);
	}

};

int main() {
	try {
		boost::asio::io_context io_context;
		TcpClient client(io_context, "127.0.0.1", "7777");

		io_context.run();
	}
	catch (std::exception& e) {
		cout << "예외맨: " << e.what() << endl;
 	}

	return 0;
}