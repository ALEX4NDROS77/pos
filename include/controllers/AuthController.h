#ifndef CONTROLLER_AUTHCONTROLLER_H
#define	CONTROLLER_AUTHCONTROLLER_H	1

#include <httplib.h>

class AuthController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void login_page(const httplib::Request& req,httplib::Response& res);
		static void vendor_login_page(const httplib::Request& req,httplib::Response& res);
		static void vendor_login_post(const httplib::Request& req,httplib::Response& res);
		static void admin_login_page(const httplib::Request& req,httplib::Response& res);
		static void admin_login_post(const httplib::Request& req,httplib::Response& res);
		static void bar_login_page(const httplib::Request& req,httplib::Response& res);
		static void bar_login_post(const httplib::Request& req,httplib::Response& res);
		static void logout(const httplib::Request& req,httplib::Response& res);
		static void homepage(const httplib::Request& req,httplib::Response& res);
};

#endif

