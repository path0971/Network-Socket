#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <iostream>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// 세션 클래스
/*
Session 클래스는 client와의 세션을 관리하며, 데이터를 주고받는 역할임
do_read()는 데이터를 비동기적으로 읽고, do_write()는 데이터를 비동기적으로 씀
*/
class Session : public std::enable_shared_from_this<Session> {
	tcp::socket socket_;
	char data_[1024];
public: 
	explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

	// 세션 시작
	void run() {
		do_read();
	}

private:
	// 데이터 읽기
	void do_read() {
		auto self = shared_from_this();
		socket_.async_read_some(net::buffer(data_), [this, self](beast::error_code ec,
			std::size_t length) {
				if (!ec) {
					do_write(length);
				}
			});
	}

	// 데이터 쓰기
	void do_write(std::size_t length) {
		auto self = shared_from_this();
		net::async_write(socket_, net::buffer(data_, length), [this, self](beast::error_code ec,
			std::size_t) { // length 안사용할 것임ㅇㅇ
				if (!ec) {
					do_read();
				}
			});
	}
};


// 리스너 클래스
/*
* Listener 클래스는 client와의 연결을 수락하는 역할로, TCP acceptor를 사용하여 연결을 수락하고,새로운 Session 객체를 생성해줌!
* do_accept()는 비동기적으로 연결을 수락함
*/
class Listener : public std::enable_shared_from_this<Listener> {
	tcp::acceptor acceptor_;
	tcp::socket socket_;

public:
	Listener(net::io_context& ioc, tcp::endpoint endpoint) : acceptor_(ioc), socket_(ioc) {
		beast::error_code ec;

		// 소켓 열기
		acceptor_.open(endpoint.protocol(), ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

		// 소켓 옵션
		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

		// 바인딩
		acceptor_.bind(endpoint, ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

		// 수신 대기
		acceptor_.listen(net::socket_base::max_listen_connections, ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

	}

	void run() {
		do_accept();
	}

private:
	// 연결 수락
	void do_accept() {
		acceptor_.async_accept(socket_, [this](beast::error_code ec) {
			if (!ec) {
				std::make_shared<Session>(std::move(socket_))->run();
			}
			do_accept();
		});
	}


};
