#include <controllers/ReportController.h>
#include <httplib.h>
#include <services/SessionService.h>
#include <services/SalesService.h>
#include <views/HtmlTemplates.h>

void ReportController::register_routes(httplib::Server& server) {
	server.Get("/reports/sales",sales_report);
}

void ReportController::sales_report(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string vendor = req.get_param_value("vendor");
	std::string product_id = req.get_param_value("product");

	auto report = SalesService::get_instance().get_sales_report(vendor,product_id);
	res.set_content(HtmlTemplates::sales_report_page(session,report,vendor,product_id),"text/html");
}
