#pragma once
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using boost::asio::ip::tcp;
using namespace std;

class Person {
private:
	std::string name_;
	int age_;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& name_;
		ar& age_;
	}

public:
	Person() = default;
	Person(const std::string& name, int age) : name_(name), age_(age) {}

	friend std::ostream& operator<< (std::ostream& os, const Person& person) {
		os << "이름: " << person.name_ << ", 나이: " << person.age_;
		return os;
	}
};


class AsyncTcpServer {
public:
	AsyncTcpServer(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
		start_accept();
	}

private: 
	tcp::acceptor acceptor_;

	void start_accept() {
		auto new_session = std::make_shared<tcp::socket>(acceptor_.get_executor());
		acceptor_.async_accept(*new_session, [this, new_session](
			const boost::system::error_code& error) {
				if (!error) {
					handle_accept(new_session);
				}
				start_accept();
			});
	}

	void handle_accept(std::shared_ptr<tcp::socket> socket) {
		auto person = std::make_shared<Person>("보리킴", 2);
		std::ostringstream oss;
		boost::archive::text_oarchive oa(oss);
		oa << *person;

		auto serialized_data = std::make_shared<std::string>(oss.str());
		boost::asio::async_write(*socket, boost::asio::buffer(*serialized_data),
			[this, socket, serialized_data](const boost::system::error_code& error,
				std::size_t) {
					if (!error) {
						cout << "클라이언트에게 데이터 전송 완료" << endl;
					}
			});
	}
};