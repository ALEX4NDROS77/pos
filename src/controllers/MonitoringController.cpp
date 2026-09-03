#include <controllers/MonitoringController.h>
#include <services/SessionService.h>
#include <views/HtmlTemplates.h>

void MonitoringController::register_routes(httplib::Server& server) {
	server.Get("/monitoring",monitoring_page);
	server.Get("/monitoring/data",monitoring_data);
}

void MonitoringController::monitoring_page(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	res.set_content(HtmlTemplates::monitoring_page(session),"text/html");
}

void MonitoringController::monitoring_data(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	res.set_content(HtmlTemplates::monitoring_body(session),"text/html");
}
