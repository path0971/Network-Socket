#include <iostream>
#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;
using namespace std;

class TcpSession : public std::enable_shared_from_this<TcpSession> {
	tcp::socket socket_;
	char data_[1024];

public:
	TcpSession(boost::asio::io_context& io_context) : socket_(io_context) {};

	tcp::socket& socket() {
		return socket_;
	}

	void start() {
		do_read();
	}

private:
	void do_read() {
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_),
			[this, self](const boost::system::error_code& error,
				std::size_t length) {
					if (!error) {
						do_write(length);
					}
			});
	}

	void do_write(std::size_t length) {
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](const boost::system::error_code& error, std::size_t) {
				if (!error) {
					do_read();
				}
			});
	}

};

class TcpServer : public std::enable_shared_from_this<TcpServer> {
public:
	TcpServer(boost::asio::io_context& io_context, short port) : acceptor_(io_context,
		tcp::endpoint(tcp::v4(), port)), io_context_(io_context) {
		start_accept();
	};

private:
	void start_accept() {
		auto new_session = std::make_shared<TcpSession>(io_context_);
		acceptor_.async_accept(new_session->socket(),
			[this, new_session](const boost::system::error_code& error) {
				if (!error) {
					new_session->start();
				}
				start_accept();
			});
	}

	tcp::acceptor acceptor_;
	boost::asio::io_context& io_context_;
};



int main() {
	try {
		boost::asio::io_context io_context;
		TcpServer server(io_context, 7777);
		io_context.run();
	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}