#include <controllers/VendorController.h>
#include <services/SessionService.h>
#include <services/VendorService.h>
#include <utils/Logger.h>
#include <views/HtmlTemplates.h>

void VendorController::register_routes(httplib::Server& server) {
	server.Get("/vendors/manage",manage_vendors);
	server.Post("/vendors/add",add_vendor);
	server.Post("/vendors/update",update_vendor);
	server.Get(R"(/vendors/delete/(.+))",delete_vendor);
	server.Get(R"(/vendors/toggle/(.+))",toggle_vendor);
	server.Post("/vendors/isle",assign_isle);
}

void VendorController::manage_vendors(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	res.set_content(HtmlTemplates::vendor_manage_page(session),"text/html");
}

void VendorController::add_vendor(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string nombre = req.get_param_value("nombre");
	std::string password = req.get_param_value("password");

	LOG_INFO("VendorController::add_vendor - Vendor add: name='" + nombre + "'");

	std::string message;
	if(VendorService::get_instance().add_vendor(nombre,password)) {
		message = "Vendedor agregado exitosamente!";
	} else {
		LOG_WARNING("VendorController::add_vendor - Vendor add failed: name='" + nombre + "'");
		message = "Error al agregar vendedor.";
	}

	res.set_content(HtmlTemplates::vendor_manage_page(session,message),"text/html");
}

void VendorController::update_vendor(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string id = req.get_param_value("id");
	std::string nombre = req.get_param_value("nombre");
	std::string password = req.get_param_value("password");
	bool activo = req.get_param_value("activo") == "1";

	std::string message;
	if(VendorService::get_instance().update_vendor(id,nombre,password,activo)) {
		message = "Vendedor actualizado!";
	} else {
		message = "Error al actualizar vendedor.";
	}

	res.set_content(HtmlTemplates::vendor_manage_page(session,message),"text/html");
}

void VendorController::delete_vendor(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string id = req.matches[1];

	LOG_INFO("VendorController::delete_vendor - Vendor delete: id='" + id + "'");

	std::string message;
	if(VendorService::get_instance().delete_vendor(id)) {
		message = "Vendedor eliminado!";
	} else {
		LOG_WARNING("VendorController::delete_vendor - Vendor delete failed: id='" + id + "'");
		message = "Error al eliminar vendedor.";
	}

	res.set_content(HtmlTemplates::vendor_manage_page(session,message),"text/html");
}

void VendorController::toggle_vendor(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string id = req.matches[1];
	auto vendor = VendorService::get_instance().get_vendor_by_id(id);

	std::string message;
	if(vendor && VendorService::get_instance().set_vendor_active(id,!vendor->activo)) {
		LOG_INFO("VendorController::toggle_vendor - Vendor toggle: id='" + id + "', active=" + std::to_string(!vendor->activo));
		message = vendor->activo ? "Vendedor desactivado!" : "Vendedor activado!";
	} else {
		LOG_WARNING("VendorController::toggle_vendor - Vendor toggle failed: id='" + id + "'");
		message = "Error al cambiar estado.";
	}

	res.set_content(HtmlTemplates::vendor_manage_page(session,message),"text/html");
}

void VendorController::assign_isle(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_role(req,res,"admin");
	if(!session) return;

	std::string id = req.get_param_value("id");
	std::string isla_id = req.get_param_value("isla_id");

	LOG_INFO("VendorController::assign_isle - Vendor isle assign: id='" + id + "', isla_id='" + isla_id + "'");

	std::string message;
	if(VendorService::get_instance().set_vendor_isle(id,isla_id)) {
		message = "Isla asignada!";
	} else {
		LOG_WARNING("VendorController::assign_isle - Vendor isle assign failed: id='" + id + "'");
		message = "Error al asignar isla.";
	}

	res.set_content(HtmlTemplates::vendor_manage_page(session,message),"text/html");
}

