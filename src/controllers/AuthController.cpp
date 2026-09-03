#include <controllers/AuthController.h>
#include <services/SessionService.h>
#include <services/VendorService.h>
#include <services/TicketService.h>
#include <views/HtmlTemplates.h>
#include <utils/Utils.h>
#include <utils/Logger.h>

void AuthController::register_routes(httplib::Server& server) {
	server.Get("/",login_page);
	server.Get("/login/vendor",vendor_login_page);
	server.Post("/login/vendor",vendor_login_post);
	server.Get("/login/admin",admin_login_page);
	server.Post("/login/admin",admin_login_post);
	server.Get("/logout",logout);
	server.Get("/home",homepage);
}

void AuthController::login_page(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(session) {
		res.set_redirect("/home");
		return;
	}
	res.set_content(HtmlTemplates::login_page(),"text/html");
}

void AuthController::vendor_login_page(const httplib::Request& req,httplib::Response& res) {
	res.set_content(HtmlTemplates::vendor_login_page(),"text/html");
}

void AuthController::vendor_login_post(const httplib::Request& req,httplib::Response& res) {
	std::string username = req.get_param_value("username");
	std::string password = req.get_param_value("password");

	LOG_INFO("AuthController::vendor_login_post - Login attempt: vendor='" + username + "'");

	if(username.empty() || password.empty()) {
		LOG_WARNING("AuthController::vendor_login_post - Empty credentials");
		res.set_content(HtmlTemplates::vendor_login_page("Ingrese usuario y contraseña"),"text/html");
		return;
	}

	auto vendor = VendorService::get_instance().authenticate(username,password);
	if(!vendor) {
		LOG_WARNING("AuthController::vendor_login_post - Invalid credentials for '" + username + "'");
		res.set_content(HtmlTemplates::vendor_login_page("Credenciales incorrectas o usuario inactivo"),"text/html");
		return;
	}

	std::string session_id = SessionService::get_instance().create_session(vendor->nombre,"vendor",vendor->isla_id);
	LOG_INFO("AuthController::vendor_login_post - Login success: vendor='" + vendor->nombre + "'");
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly");
	res.set_redirect("/home");
}

void AuthController::admin_login_page(const httplib::Request& req,httplib::Response& res) {
	res.set_content(HtmlTemplates::admin_login_page(),"text/html");
}

void AuthController::admin_login_post(const httplib::Request& req,httplib::Response& res) {
	std::string username = req.get_param_value("username");
	std::string password = req.get_param_value("password");

	LOG_INFO("AuthController::admin_login_post - Login attempt: admin='" + username + "'");

	if(!SessionService::get_instance().validate_admin_password(password)) {
		LOG_WARNING("AuthController::admin_login_post - Login failed: invalid admin password");
		res.set_content(HtmlTemplates::admin_login_page("Contraseña incorrecta!"),"text/html");
		return;
	}

	std::string session_id = SessionService::get_instance().create_session(username,"admin");
	LOG_INFO("AuthController::admin_login_post - Login success: admin='" + username + "'");
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly");
	res.set_redirect("/home");
}

void AuthController::logout(const httplib::Request& req,httplib::Response& res) {
	std::string session_id = Utils::get_session_id_from_cookie(req);
	if(!session_id.empty()) {
		LOG_INFO("AuthController::logout - Logout: session_id=" + session_id.substr(0,8) + "...");
		SessionService::get_instance().destroy_session(session_id);
	}
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly; Max-Age=0");
	res.set_redirect("/");
}

void AuthController::homepage(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_session(req,res);
	if(!session) return;
	auto tickets = TicketService::get_instance().get_pending_tickets();
	res.set_content(HtmlTemplates::homepage(session,tickets),"text/html");
}
