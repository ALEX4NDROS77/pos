#include <controllers/IsleController.h>
#include <services/IsleService.h>
#include <services/SessionService.h>
#include <utils/Logger.h>
#include <views/HtmlTemplates.h>

void IsleController::register_routes(httplib::Server& server) {
	server.Get("/isles/manage",manage_isles);
	server.Post("/isles/add",add_isle);
	server.Get(R"(/isles/delete/(.+))",delete_isle);
}

void IsleController::manage_isles(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	res.set_content(HtmlTemplates::isle_manage_page(session),"text/html");
}

void IsleController::add_isle(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string nombre = req.get_param_value("nombre");

	LOG_INFO("IsleController::add_isle - Isle add: name='" + nombre + "'");

	std::string message;
	if(IsleService::get_instance().add_isle(nombre)) {
		message = "Isla agregada exitosamente!";
	} else {
		LOG_WARNING("IsleController::add_isle - Isle add failed: name='" + nombre + "'");
		message = "Error al agregar isla.";
	}

	res.set_content(HtmlTemplates::isle_manage_page(session,message),"text/html");
}

void IsleController::delete_isle(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string id = req.matches[1];

	LOG_INFO("IsleController::delete_isle - Isle delete: id='" + id + "'");

	std::string message;
	if(auto error = IsleService::get_instance().delete_isle(id)) {
		LOG_WARNING("IsleController::delete_isle - Isle delete blocked: id='" + id + "', reason='" + *error + "'");
		message = *error;
	} else {
		message = "Isla eliminada!";
	}

	res.set_content(HtmlTemplates::isle_manage_page(session,message),"text/html");
}
