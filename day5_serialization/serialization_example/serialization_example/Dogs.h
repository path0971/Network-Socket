#pragma once
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

class Dogs {
public:
	Dogs() = default;
	Dogs(const std::string& name, int age) : name_(name), age_(age) {};

	friend std::ostream& operator << (std::ostream& os, const Dogs& dog) {
		os << "이름: " << dog.name_ << ", 나이: " << dog.age_;
		return os;
	}

private:
	std::string name_;
	int age_;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar& name_;
		ar& age_;
	}
};