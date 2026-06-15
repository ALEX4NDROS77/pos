#include <controllers/InventoryController.h>
#include <services/InventoryService.h>
#include <services/SessionService.h>
#include <utils/Logger.h>
#include <utils/Utils.h>
#include <views/HtmlTemplates.h>

static std::string get_form_value(const httplib::Request& req, const std::string name) {
	if(req.has_file(name)) {
		return req.get_file_value(name).content;
	}
	return req.get_param_value(name);
}

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

	LOG_INFO("InventoryController::add_product: Processing request");

	std::string name = get_form_value(req,"name");
	std::string price_str = get_form_value(req,"price");
	std::string quantity_str = get_form_value(req,"quantity");

	double price = 0;
	int quantity = 0;

	try {
		price = std::stod(price_str);
		quantity = std::stoi(quantity_str);
	} catch (std::exception& e) {
		LOG_ERROR("InventoryController::add_product: Failed to parse - " + std::string(e.what()));
		res.set_content(HtmlTemplates::inventory_manage_page(session,"Error: datos invalidos"),"text/html");
		return;
	}

	LOG_DEBUG("InventoryController::add_product: name=" + name + ", price=" + std::to_string(price) + ", quantity=" + std::to_string(quantity));

	std::string imagen = get_form_value(req,"imagen");
	if(!imagen.empty()) {
		LOG_INFO("InventoryController::add_product: Compressed image received, length=" + std::to_string(imagen.size()));
	} else {
		LOG_DEBUG("InventoryController::add_product: No image uploaded");
	}

	std::string message;
	if(InventoryService::get_instance().add_product(name,price,quantity,imagen)) {
		message = "Producto agregado exitosamente!";
		LOG_INFO("InventoryController::add_product: Product added successfully");
	} else {
		message = "Error al agregar producto.";
		LOG_ERROR("InventoryController::add_product: Failed to add product");
	}

	res.set_content(HtmlTemplates::inventory_manage_page(session,message),"text/html");
}

void InventoryController::update_product(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "admin") {
		res.set_redirect("/");
		return;
	}

	LOG_INFO("InventoryController::update_product: Processing request");

	std::string id = get_form_value(req,"id");
	std::string name = get_form_value(req,"name");
	std::string price_str = get_form_value(req,"price");
	std::string quantity_str = get_form_value(req,"quantity");

	double price = 0;
	int quantity = 0;

	try {
		price = std::stod(price_str);
		quantity = std::stoi(quantity_str);
	} catch(const std::exception& e) {
		LOG_ERROR("InventoryController::update_product: Failed to parse - " + std::string(e.what()));
		res.set_content(HtmlTemplates::inventory_manage_page(session,"Error: datos invalidos"),"text/html");
		return;
	}

	LOG_DEBUG("InventoryController::update_product: id=" + id + ", name=" + name + ", price=" + std::to_string(price) + ", quantity=" + std::to_string(quantity));

	std::string imagen = get_form_value(req,"imagen");
	if(!imagen.empty()) {
		LOG_INFO("InventoryController::update_product: New compressed image, length=" + std::to_string(imagen.size()));
	} else {
		LOG_DEBUG("InventoryController::update_product: No new image, keeping existing");
	}

	std::string message;
	if(InventoryService::get_instance().update_product(id,name,price,quantity,imagen)) {
		message = "Producto actualizado";
		LOG_INFO("InventoryController::update_product: Product updated successfully");
	} else {
		message = "Error al actualizar producto.";
		LOG_ERROR("InventoryController::update_product: Failed to update product");
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
