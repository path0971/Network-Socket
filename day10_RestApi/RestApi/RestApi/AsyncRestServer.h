#pragma once
#include "UserManager.h"
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
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// 클라이언트와 세션을 관리하는 클래스로, HTTP 요청을 처리하고, 응답 반환
class session : public std::enable_shared_from_this<session> {
	tcp::socket socket_;
	beast::flat_buffer buffer_;
	http::request<http::string_body> req_;
	std::shared_ptr<UserManager> userManager_;
public:
	session(tcp::socket socket, std::shared_ptr<UserManager> userManager) : socket_(std::move(socket)), userManager_(std::move(userManager)) {}

	void run() {
		do_read();
	}

private:
	void do_read() {
		auto self = shared_from_this();
		http::async_read(socket_, buffer_, req_, [self](beast::error_code ec,
			std::size_t bytes_transferred) {
				boost::ignore_unused(bytes_transferred);
				if (!ec) {
					self->handle_request();
				}
			});
	}

	void handle_request() {
		auto const reply = [this](http::status status, std::string body) {
			http::response<http::string_body> res{ status, req_.version() };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/plain");
			res.keep_alive(req_.keep_alive());
			res.body() = std::move(body);
			res.prepare_payload();
			return res;
			};

		auto const bad_request = [this](beast::string_view why) {
			http::response<http::string_body> res{
			http::status::bad_request, req_.version() };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/html");
			res.keep_alive(req_.keep_alive());
			res.body() = std::string(why);
			res.prepare_payload();
			return res;
			};

		auto const not_found = [this](beast::string_view target) {
			http::response<http::string_body> res{
				http::status::not_found, req_.version() };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/html");
			res.keep_alive(req_.keep_alive());
			res.body() = "오옹 " + std::string(target) + " 못찾음ㅠㅠ";
			res.prepare_payload();
			return res;
			};

		auto const server_error = [this](beast::string_view what) {
			http::response<http::string_body> res{
			http::status::internal_server_error, req_.version()
			};
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(http::field::content_type, "text/html");
			res.keep_alive(req_.keep_alive());
			res.body() = "오류오류: " + std::string(what) + "";
			res.prepare_payload();
			return res;
			};

		try {
			if (req_.method() == http::verb::get) {
				if (req_.target() == "/users") {
					// 모든 사용자 조회 로직 
					do_write(reply(http::status::ok, "모든 사용자 목록"));
					return;
				}
				else if (req_.target().starts_with("/users/")) {
					// 특정 사용자 조회 로직 
					std::string id = std::string(req_.target().substr(7)); // /users/ 이후
					do_write(reply(http::status::ok, "사용자 조회: id=" + id));
					return;
				}
			}
			else if (req_.method() == http::verb::post) {
				if (req_.target() == "/users") {
					// 새로운 사용자 생성 로직 
					do_write(reply(http::status::created, "사용자 생성됨"));
					return;
				}
			}
			else if (req_.method() == http::verb::delete_) {
				if (req_.target().starts_with("/users/")) {
					// 특정 사용자 삭제 로직 
					std::string id = std::string(req_.target().substr(7));
					do_write(reply(http::status::ok, "사용자 삭제됨: id=" + id));
					return;
				}
			}
		}
		catch (std::exception const& e) {
			do_write(server_error(e.what()));
			return;
		}

		do_write(reply(http::status::bad_request, "fucked request"));
	}

	void do_write(http::response<http::string_body>&& res) {
		auto self = shared_from_this();
		http::async_write(socket_, res, [self](beast::error_code ec,
			std::size_t bytes_transferred) {
				boost::ignore_unused(bytes_transferred);
				self->socket_.shutdown(tcp::socket::shutdown_send, ec);
				self->socket_.close();
			});
	}
};


// HTTP 서버 클래스로, HTTP 요청을 비동기 처리하고 응답 반환
class AsyncRestServer : public std::enable_shared_from_this<AsyncRestServer> {
private:
	net::io_context& ioc_;
	tcp::acceptor acceptor_;
	std::shared_ptr<UserManager> userManager_;

	void do_accept() {
		acceptor_.async_accept(
			net::make_strand(ioc_), [self = shared_from_this()](beast::error_code ec, tcp::socket socket) {
				if (!ec) {
					std::make_shared<session>(std::move(socket), self->userManager_)->run();
				}
				self->do_accept();
			});
	}



public:
	AsyncRestServer(net::io_context& ioc, tcp::endpoint endpoint) : ioc_(ioc), acceptor_(net::make_strand(ioc)),
		userManager_(std::make_shared<UserManager>()) {
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
};