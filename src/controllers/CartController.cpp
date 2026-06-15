#include <controllers/CartController.h>
#include <services/SessionService.h>
#include <services/CartService.h>
#include <services/SalesService.h>
#include <views/HtmlTemplates.h>

void CartController::register_routes(httplib::Server& server) {
	server.Get("/cart",cart_page);
	server.Post("/cart/add",add_to_cart);
	server.Post("/cart/update",update_cart_item);
	server.Get(R"(/cart/remove/(.+))",remove_from_cart);
	server.Get(R"(/cart/update/([^/]+/(\d+)))",update_cart_item_get);
	server.Get("/cart/clear",clear_cart);
	server.Get("/cart/confirm",confirm_page);
	server.Post("/cart/checkout",checkout);
	server.Get(R"(/cart/checkout/([ETC]))",checkout_get);
}

void CartController::cart_page(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	res.set_content(HtmlTemplates::cart_page(session),"text/html");
}

void CartController::checkout_get(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}

	char payment_method = req.matches[1].str()[0];

	auto order_id = SalesService::get_instance().checkout(session,payment_method);

	if(order_id && order_id->find("ORD") != std::string::npos) {
		res.set_content(HtmlTemplates::cart_confirm_page(session,"Venta completada! ID: " + *order_id),"text/html");
	} else if(order_id) {
		res.set_content(HtmlTemplates::cart_confirm_page(session,*order_id),"text/html");
	} else {
		res.set_content(HtmlTemplates::cart_confirm_page(session,"Error al procesar venta."),"text/html");
	}
}

void CartController::add_to_cart(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	std::string product_id = req.get_param_value("product_id");
	int quantity = std::stoi(req.get_param_value("quantity"));

	std::string message;
	if(CartService::get_instance().add_to_cart(session,product_id,quantity)) {
		message = "Producto agregado al carrito!";
	} else {
		message = "Error al agregar producto.";
	}

	res.set_content(HtmlTemplates::cart_page(session,message),"text/html");
}

void CartController::update_cart_item(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}

	std::string product_id = req.get_param_value("product_id");
	int quantity = std::stoi(req.get_param_value("quantity"));

	CartService::get_instance().update_cart_item(session,product_id,quantity);
	res.set_content(HtmlTemplates::cart_page(session,"Carrito actualizado!"),"text/html");
}

void CartController::update_cart_item_get(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}

	std::string product_id = req.matches[1].str();
	int quantity = std::stoi(req.matches[2].str());

	CartService::get_instance().update_cart_item(session,product_id,quantity);
	res.set_redirect("/cart");
}

void CartController::remove_from_cart(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	
	std::string product_id = req.matches[1];
	CartService::get_instance().remove_from_cart(session,product_id);
	res.set_content(HtmlTemplates::cart_page(session,"Producto eliminado!"),"text/html");
}

void CartController::clear_cart(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	
	CartService::get_instance().clear_cart(session);
	res.set_content(HtmlTemplates::cart_page(session,"Carrito vaciado!"),"text/html");
}

void CartController::confirm_page(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	res.set_content(HtmlTemplates::cart_confirm_page(session),"text/html");
}

void CartController::checkout(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	
	std::string payment_method_str = req.get_param_value("payment_method");
	char payment_method = payment_method_str.empty() ? 'E' : payment_method_str[0];

	auto order_id = SalesService::get_instance().checkout(session,payment_method);

	if(order_id && order_id->find("ORD") != std::string::npos) {
		res.set_content(HtmlTemplates::cart_confirm_page(session,"Venta completada! ID: " + *order_id),"text/html");
	} else if(order_id) {
		res.set_content(HtmlTemplates::cart_confirm_page(session,*order_id),"text/html");
	} else {
		res.set_content(HtmlTemplates::cart_confirm_page(session,"Error al procesar venta."),"text/html");
	}
}
