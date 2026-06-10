#include <controllers/ReportController.h>
#include <services/SessionService.h>
#include <services/SalesService.h>
#include <views/HtmlTemplates.h>

void ReportController::register_routes(httplib::Server& server) {
	server.Get("/reports/sales",sales_report);
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
