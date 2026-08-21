#include <controllers/ReportController.h>
#include <httplib.h>
#include <services/SessionService.h>
#include <services/SalesService.h>
#include <views/HtmlTemplates.h>

void ReportController::register_routes(httplib::Server& server) {
	server.Get("/reports/sales",sales_report);
	server.Get(R"(/reports/sales/vendor/(.+))",sales_report_by_vendor);
}

void ReportController::sales_report(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	auto report = SalesService::get_instance().get_sales_report();
	res.set_content(HtmlTemplates::sales_report_page(session,report),"text/html");
}

void ReportController::sales_report_by_vendor(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	std::string vendor = req.matches[1].str();
	auto report = SalesService::get_instance().get_sales_report_by_vendor(vendor);
	res.set_content(HtmlTemplates::sales_report_page(session,report,vendor),"text/html");
}
