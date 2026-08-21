#ifndef CONTROLLER_REPORTCONTROLLER_H
#define	CONTROLLER_REPORTCONTROLLER_H	1

#include <httplib.h>

class ReportController {
	public:
		static void register_routes(httplib::Server& server);

	private:
		static void sales_report(const httplib::Request& req,httplib::Response& res);
		static void sales_report_by_vendor(const httplib::Request& req,httplib::Response& res);
};

#endif

