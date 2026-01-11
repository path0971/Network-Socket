#pragma once
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/* 전반적인 아키텍쳐
listener
 └─ 클라이언트 연결 수락 (async_accept)
	 └─ session 생성
		 └─ HTTP 요청 읽기 (async_read)
			 └─ 요청 파싱 및 처리
				 └─ HTTP 응답 생성
					 └─ 응답 쓰기 (async_write)
						 └─ 소켓 종료

서버는 listener --> session 구조로 이루어져 있으며, 각 클라이언트 연결은 session 객체 하나가 처리!
*/



// 세션 클래스
class session : public std::enable_shared_from_this<session> {
	// 세션은 비동기 처리 동안 살아야하므로, shared_ptr 생명주기 사용!
	tcp::socket socket_;
	beast::flat_buffer buffer_;
	http::request<http::string_body> req_;

public:
	explicit session(tcp::socket socket) : socket_(std::move(socket)) {}
	void run() {
		do_read();
	}

private:
	void do_read() {
		auto self = shared_from_this();
		// 람다 내에서 self(shared_ptr)를 캡쳐하여 session 파괴 방지!
		http::async_read(socket_, buffer_, req_,
			[self](beast::error_code ec, std::size_t bytes_transferred) {
				boost::ignore_unused(bytes_transferred);
				if (!ec) {
					self->handle_request();
				}
			});
	}

	void handle_request() {
		auto const bad_request = [&req = req_](beast::string_view why) {
			http::response<http::string_body> res{ http::status::bad_request, req.version() };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/html");
			res.keep_alive(req.keep_alive());
			res.body() = std::string(why);
			res.prepare_payload();
			return res;
			};

		// GET,HEAD가 아니면 400 오류 반환!
		if (req_.method() != http::verb::get && req_.method() != http::verb::head) {
			return do_write(bad_request("모르는 HTTP method"));
		}

		// URL(target) 검사
		if (req_.target() != "/index.html") {
			return do_write(bad_request("없는 페이지"));
		}

		/* 정상 응답맨 생성
		http::string_body::value_type body = "Oh shit!";
		auto const size = body.size();

		http::response<http::string_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, req_.version())
		};
		*/

		/* HTTP 헤더 구성!
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.content_length(size);
		res.keep_alive(req_.keep_alive());
		return do_write(std::move(res));
		*/
	}

	void do_write(http::response<http::string_body>&& res) {
		auto self = shared_from_this();
		http::async_write(socket_, res, [self](beast::error_code ec, std::size_t bytes_transferred) {
			self->socket_.shutdown(tcp::socket::shutdown_send, ec);
			self->socket_.close();
			}
		);
	}
};

// 리스너 클래스
class listener : public std::enable_shared_from_this<listener> {
	net::io_context& ioc_;
	tcp::acceptor acceptor_;

public:
	// TCP 서버의 절차: open --> options --> bind --> listen
	listener(net::io_context& ioc, tcp::endpoint endpoint) : ioc_(ioc), acceptor_(net::make_strand(ioc)) {
		beast::error_code ec;

		acceptor_.open(endpoint.protocol(), ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

		acceptor_.bind(endpoint, ec);
		if (ec) {
			std::cerr << "오류맨: " << ec.message() << std::endl;
			return;
		}

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
	void do_accept() {
		auto self = shared_from_this();
		acceptor_.async_accept(net::make_strand(ioc_), [self](beast::error_code ec, tcp::socket socket) {
			if (!ec) {
				std::make_shared<session>(std::move(socket))->run();
			}
			self->do_accept(); // 다시 do_accept()로 새로운 연결 대기! 이를 통해 많은 클라이언트 처리가 가능!
			});
	}

};