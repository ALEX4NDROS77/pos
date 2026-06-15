#ifndef CONTROLLERS_BARCONTROLLER_H
#define	CONTROLLERS_BARCONTROLLER_H	1

#include <httplib.h>

class BarController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void bar_screen(const httplib::Request& req,httplib::Response& res);
		static void update_ticket_status(const httplib::Request& req,httplib::Response& res);
		static void get_tickets_json(const httplib::Request& req,httplib::Response& res);
};

#endif

