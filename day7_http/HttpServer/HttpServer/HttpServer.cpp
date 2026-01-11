#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <vector>

// namespace aliases
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
// Type alias
using tcp = net::ip::tcp;

class session : public std::enable_shared_from_this<session> {
	tcp::socket socket_;
	beast::flat_buffer buffer_;
	http::request<http::string_body> req_;

public:
	explicit session(tcp::socket&& socket) : socket_(std::move(socket)) {}

	void run() { 
		do_read(); 
	}

private: 
	void do_read() {
		auto self = shared_from_this();
		http::async_read(socket_, buffer_, req_, [self](beast::error_code ec, std::size_t bytes_transferred) {
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

		if (req_.method() != http::verb::get && req_.method() != http::verb::head) {
			return do_write(bad_request("Unknown HTTP-method"));
		}

		if (req_.target() != "/index.html") {
			return do_write(bad_request("Illegal request-target"));
		}

		http::string_body::value_type body = "Hello, World!";
		auto const size = body.size();

		http::response<http::string_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, req_.version())
		};
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.content_length(size);
		res.keep_alive(req_.keep_alive());
		return do_write(std::move(res));
	}


	/*
	lvalue reference를 받으면 응답 객체가 함수 스택에 있거나, 아직 scope 바깥으로 나가지 않았는데 비동기 작업(쓰기)이 끝나기 전에 response가 사라질 위험이 있다.
	response를 보호하기 위해 rvalue reference로 move semantics를 사용한다!
	*/
	void do_write(http::response<http::string_body>&& res) {
		auto self = shared_from_this();
		http::async_write(socket_, res, [self](beast::error_code ec, std::size_t bytes_transferred) {
			self->socket_.shutdown(tcp::socket::shutdown_send, ec);
			self->socket_.close();
			});
	}

};


class listener : public std::enable_shared_from_this<listener> {
	net::io_context& ioc_;
	tcp::acceptor acceptor_;

public:
	listener(net::io_context& ioc, tcp::endpoint endpoint) : ioc_(ioc), acceptor_(ioc) {
		beast::error_code ec;

		acceptor_.open(endpoint.protocol(), ec);
		if (ec) {
			std::cerr << "오류: " << ec.message() << std::endl;
			return;
		}

		acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec) {
			std::cerr << "오류: " << ec.message() << std::endl;
			return;
		}

		acceptor_.bind(endpoint, ec);
		if (ec) {
			std::cerr << "오류: " << ec.message() << std::endl;
			return;
		}

		acceptor_.listen(net::socket_base::max_listen_connections, ec);

		if (ec) {
			std::cerr << "오류: " << ec.message() << std::endl;
			return;
		}
	}

	void run() {
		do_accept();
	}

private:
	void do_accept() {
		auto self = shared_from_this();
		acceptor_.async_accept([self](beast::error_code ec, tcp::socket socket) {
			if (!ec) {
				std::make_shared<session>(std::move(socket))->run();
			}
			self->do_accept();
			});
	}
			
};

int main() {
	try{
		auto const address = net::ip::make_address("0.0.0.0");
		auto const port = static_cast<unsigned short>(std::atoi("7777"));
		net::io_context ioc{ 1 };

		auto listener_ptr = std::make_shared<listener>(ioc, tcp::endpoint{ address, port });
		listener_ptr->run();
		ioc.run();
	}
	catch (std::exception const& e) {
		std::cerr << "예외맨: " << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}