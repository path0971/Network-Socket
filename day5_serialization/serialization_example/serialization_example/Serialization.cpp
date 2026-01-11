#include <iostream>
#include <sstream>
#include "Dogs.h"

int main() {
	Dogs dog1("보리킴", 2);

	// 객체를 문자열 스트림으로 직렬화
	std::ostringstream oss;
	boost::archive::text_oarchive oa(oss);
	oa << dog1;

	std::string serializedData = oss.str();
	std::cout << "직렬화된 데이터: " << serializedData << std::endl;

	// 문자열 스트림에서 객체를 역직렬화
	std::istringstream iss(serializedData);
	boost::archive::text_iarchive ia(iss);
	Dogs dog2;
	ia >> dog2;

	std::cout << "역직렬화된 객체: " << dog2 << std::endl;

	return 0;
}