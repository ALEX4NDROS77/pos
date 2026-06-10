#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <utils/Utils.h>

namespace Utils {
	std::string generate_session_id() {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<> dis(0,15);
		const char* hex = "0123456789ABCDEF";

		std::string id;
		for(int i = 0;i < 32;++i) {
			id += hex[dis(gen)];
		}
		return id;
	}

	std::string generate_uuid() {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<> dis(0,15);
		static std::uniform_int_distribution<> dis2(8,11);
		const char* hex = "0123456789ABCDEF";

		std::string uuid;
		for(int i = 0;i < 36;++i) {
			if(i == 8 || i == 13 || i == 18 || i == 23) {
				uuid += '-';
			} else if(i == 14) {
				uuid += '4';
			} else if(i == 19) {
				uuid += hex[dis2(gen)];
			} else {
				uuid += hex[dis(gen)];
			}
		}

		return uuid;
	}

	std::string get_timestamp() {
		auto now = std::time(nullptr);
		std::ostringstream oss;
		oss << std::put_time(std::localtime(&now),"%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	std::string get_current_time() {
		auto now = std::time(nullptr);
		std::ostringstream oss;
		oss << std::put_time(std::localtime(&now),"%m/%d %H:%M");
		return oss.str();
	}

	std::string get_session_id_from_cookie(const httplib::Request& req) {
		if(req.has_header("Cookie")) {
			std::string cookie = req.get_header_value("Cookie");
			size_t pos = cookie.find("session_id=");
			if(pos != std::string::npos) {
				size_t start = pos + 11;
				size_t end = cookie.find(';',start);
				if(end == std::string::npos) end = cookie.length();
				return cookie.substr(start,end - start);
			}
		}
		return "";
	}
}

