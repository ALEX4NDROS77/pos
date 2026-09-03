#ifndef CONTROLLER_TRANSFERCONTROLLER_H
#define	CONTROLLER_TRANSFERCONTROLLER_H	1

#include <httplib.h>

class TransferController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void transfer_page(const httplib::Request& req,httplib::Response& res);
		static void create_transfer(const httplib::Request& req,httplib::Response& res);
};

#endif

