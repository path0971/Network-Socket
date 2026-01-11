#pragma once
#include "User.h"
#include <unordered_map>
#include <mutex>
#include <memory>

// 사용자 정보 관리용 클래스로, 사용자를 생성, 조회, 삭제하는 기능!
class UserManager {
public:
	std::shared_ptr<User> createUser(const std::string& name,
		int age) {
		std::lock_guard<std::mutex> lock(mutex_);
		auto user = std::make_shared<User>(name, age);
		users_[user->getId()] = user;
		return user;
	}

	std::shared_ptr<User> getUser(const std::string& id) {
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = users_.find(id);
		if (it != users_.end()) {
			return it->second;
		}
		return nullptr;
	}

	bool deleteUser(const std::string& id) {
		std::lock_guard<std::mutex> lock(mutex_);
		return users_.erase(id) > 0; // 삭제된 요소 수 반환
	}
 
private:
	std::unordered_map<std::string, std::shared_ptr<User>> users_;
	std::mutex mutex_;
};