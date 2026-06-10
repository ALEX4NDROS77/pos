#include <controllers/InventoryController.h>
#include <services/SessionService.h>
#include <services/InventoryService.h>
#include <views/HtmlTemplates.h>

void InventoryController::register_routes(httplib::Server& server) {
	server.Get("/inventory/view",view_inventory);
	server.Get("/inventory/manage",manage_inventory);
	server.Post("/inventory/add",add_product);
	server.Post("/inventory/update",update_product);
	server.Get(R"(/inventory/delete/(.+))",delete_product);
}

void InventoryController::view_inventory(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	res.set_content(HtmlTemplates::inventory_view_page(session),"text/html");
}

void InventoryController::manage_inventory(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}
	res.set_content(HtmlTemplates::inventory_manage_page(session),"text/html");
}

void InventoryController::add_product(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	std::string name = req.get_param_value("name");
	double price = std::stod(req.get_param_value("price"));
	int quantity = std::stoi(req.get_param_value("quantity"));

	std::string message;
	if(InventoryService::get_instance().add_product(name,price,quantity)) {
		message = "Producto agregado exitosamente!";
	} else {
		message = "Error al agregar producto.";
	}

	res.set_content(HtmlTemplates::inventory_manage_page(session,message),"text/html");
}

void InventoryController::update_product(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	std::string id = req.get_param_value("id");
	std::string name = req.get_param_value("name");
	double price = std::stod(req.get_param_value("price"));
	int quantity = std::stoi(req.get_param_value("quantity"));

	std::string message;
	if(InventoryService::get_instance().update_product(id,name,price,quantity)) {
		message = "Producto actualizado";
	} else {
		message = "Error al actualizar producto.";
	}

	res.set_content(HtmlTemplates::inventory_manage_page(session,message),"text/html");
}

void InventoryController::delete_product(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	std::string id = req.matches[1];
	
	std::string message;
	if(InventoryService::get_instance().delete_product(id)) {
		message = "Producto eliminado";
	} else {
		message = "Error al eliminar producto.";
	}

	res.set_content(HtmlTemplates::inventory_manage_page(session,message),"text/html");
}
