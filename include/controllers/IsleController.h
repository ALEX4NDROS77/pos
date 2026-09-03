#ifndef CONTROLLER_ISLECONTROLLER_H
#define	CONTROLLER_ISLECONTROLLER_H	1

#include <httplib.h>

class IsleController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void manage_isles(const httplib::Request& req,httplib::Response& res);
		static void add_isle(const httplib::Request& req,httplib::Response& res);
		static void delete_isle(const httplib::Request& req,httplib::Response& res);
};

#endif

