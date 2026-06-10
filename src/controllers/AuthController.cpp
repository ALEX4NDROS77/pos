#include <controllers/AuthController.h>
#include <services/SessionService.h>
#include <views/HtmlTemplates.h>
#include <utils/Utils.h>

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
	if(username.empty()) {
		res.set_content(HtmlTemplates::vendor_login_page(),"text/html");
		return;
	}

	std::string session_id = SessionService::get_instance().create_session(username,"vendor");
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly");
	res.set_redirect("/home");
}

void AuthController::admin_login_page(const httplib::Request& req,httplib::Response& res) {
	res.set_content(HtmlTemplates::admin_login_page(),"text/html");
}

void AuthController::admin_login_post(const httplib::Request& req,httplib::Response& res) {
	std::string username = req.get_param_value("username");
	std::string password = req.get_param_value("password");

	if(!SessionService::get_instance().validate_admin_password(password)) {
		res.set_content(HtmlTemplates::admin_login_page("Contraseña incorrecta!"),"text/html");
		return;
	}

	std::string session_id = SessionService::get_instance().create_session(username,"admin");
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly");
	res.set_redirect("/home");
}

void AuthController::logout(const httplib::Request& req,httplib::Response& res) {
	std::string session_id = Utils::get_session_id_from_cookie(req);
	if(!session_id.empty()) {
		SessionService::get_instance().destroy_session(session_id);
	}
	res.set_header("Set-Cookie","session_id=" + session_id + "; Path=/; HttpOnly; Max-Age=0");
	res.set_redirect("/");
}

void AuthController::homepage(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session) {
		res.set_redirect("/");
		return;
	}
	res.set_content(HtmlTemplates::homepage(session),"text/html");
}
