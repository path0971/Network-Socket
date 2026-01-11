#pragma once
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// 사용자 정보를 나타내는 클래스로, 이름과 나이를 포함함
class User {
public:
	User(const std::string& name, int age) : id_(boost::uuids::random_generator()()),
		name_(name), age_(age) {}

	std::string getId() const {
		return boost::uuids::to_string(id_);
	}

	std::string getName() const {
		return name_;
	}

	int getAge() const {
		return age_;
	}

	void setName(const std::string& name) {
		name_ = name;
	}

	void setAge(int age) {
		age_ = age;
	}

private:
	boost::uuids::uuid id_;
	std::string name_;
	int age_;
};