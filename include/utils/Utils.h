#ifndef UTILS_H
#define	UTILS_H	1

#include <string>
#include <httplib.h>

namespace Utils {
	std::string generate_session_id();
	std::string generate_uuid();
	std::string get_timestamp();
	std::string get_current_time();
	std::string get_session_id_from_cookie(const httplib::Request& req);
}

#endif

