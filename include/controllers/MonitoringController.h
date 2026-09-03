#ifndef CONTROLLER_MONITORINGCONTROLLER_H
#define	CONTROLLER_MONITORINGCONTROLLER_H	1

#include <httplib.h>

class MonitoringController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void monitoring_page(const httplib::Request& req,httplib::Response& res);
		static void monitoring_data(const httplib::Request& req,httplib::Response& res);
};

#endif

