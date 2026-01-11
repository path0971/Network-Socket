#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <iostream>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {

public:
	explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

	void run() {
		do_read();
	}

private:
	tcp::socket socket_;
	char data_[1024];

	void do_read() {
		auto self = shared_from_this();
		socket_.async_read_some(net::buffer(data_), [this, self](beast::error_code ec,
			std::size_t length) {
				if (!ec) {
					do_write(length);
				}
			}
		);
	}

	void do_write(std::size_t length) {
		auto self = shared_from_this();
		net::async_write(socket_, net::buffer(data_, length), [this, self](beast::error_code ec,
			size_t length) {
				if (!ec) {
					do_read();
				}
			});
	}
};


class Listener : public std::enable_shared_from_this<Listener> {
public:
	Listener(net::io_context& ioc, tcp::endpoint endpoint) : acceptor_(ioc), socket_(ioc) {
		beast::error_code ec;

		acceptor_.open(endpoint.protocol(), ec);
		if (ec) {
			std::cerr << "에러맨: " << ec.message() << std::endl;
			return;
		}

		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec) {
			std::cerr << "에러맨: " << ec.message() << std::endl;
			return;
		}

		acceptor_.bind(endpoint, ec);
		if (ec) {
			std::cerr << "에러맨: " << ec.message() << std::endl;
			return;
		}

		acceptor_.listen(net::socket_base::max_listen_connections, ec);
		if (ec) {
			std::cerr << "에러맨: " << ec.message() << std::endl;
			return;
		}
	}

	void run() {
		do_accept();
	}

private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;

	void do_accept() {
		acceptor_.async_accept(socket_, [this](beast::error_code ec) {
			if (!ec) {
				std::make_shared<Session>(std::move(socket_))->run();
			}
			do_accept();
			});
	}

};