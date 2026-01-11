#pragma once
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>

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
	Person(const string& name, int age) : name_(name), age_(age) {}

	friend ostream& operator<<(ostream& os, const Person& person) {
		os << "이름: " << person.name_ << ", 나이: " << person.age_;
		return os;
	}
};


class AsyncTcpClient {
public:
	AsyncTcpClient(boost::asio::io_context& io_context, const string& host,
		const string& service) : resolver_(io_context), socket_(io_context) {
		connect(host, service);
	}

private:
	tcp::socket socket_;
	tcp::resolver resolver_;
	char data_[1024];

	void connect(const string& host, const string& service) {
		auto endpoints = resolver_.resolve(host, service);
		boost::asio::async_connect(socket_, endpoints,
			[this](const boost::system::error_code& error, const tcp::endpoint&) {
				if (!error) {
					do_read();
				}
			});
	}

	void do_read() {
		boost::asio::async_read(socket_, boost::asio::buffer(data_),
			[this](const boost::system::error_code& error, std::size_t length) {
				if (!error) {
					std::istringstream iss(string(data_, length));
					boost::archive::text_iarchive ia(iss);
					Person person;
					ia >> person;

					std::cout << "수신한 데이터: " << person << endl;
				}
			});
	}

};