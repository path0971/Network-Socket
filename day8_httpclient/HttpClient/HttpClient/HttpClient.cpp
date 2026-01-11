#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std;

int main() {
	try {
		auto const host = "www.example.com";
		auto const port = "80";
		auto const target = "/index.html";
		int version = 11; // HTTP/1.1

		// IO context 생성
		net::io_context ioc;

		// Resolver와 socket 생성
		tcp::resolver resolver(ioc);
		auto const results = resolver.resolve(host, port);

		beast::tcp_stream stream(ioc);
		stream.connect(results);

		// http 요청 생성
		http::request<http::string_body> req{http::verb::get, target, version};
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		// 요청 전송
		http::write(stream, req);

		// 응답용 버퍼
		beast::flat_buffer buffer;

		// 응답 객체 생성
		http::response<http::dynamic_body> res;
		http::read(stream, buffer, res);

		// 응답 출력
		cout << res << endl;

		// 연결 종료
		beast::error_code ec;
		stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		if (ec && ec != beast::errc::not_connected) {
			throw beast::system_error{ ec };
		}
	}
	catch (std::exception const& e) {
		cerr << "예외맨: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	return 0;
}
