#ifndef	CONTROLLERS_TICKETCONTROLLER_H
#define	CONTROLLERS_TICKETCONTROLLER_H	1

#include <httplib.h>

class TicketController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void complete_ticket(const httplib::Request& req,httplib::Response& res);
};

#endif
