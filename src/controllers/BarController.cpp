#include <controllers/BarController.h>
#include <services/SessionService.h>
#include <services/TicketService.h>
#include <views/HtmlTemplates.h>
#include <utils/Logger.h>
#include <sstream>

void BarController::register_routes(httplib::Server& server) {
	server.Get("/bar",bar_screen);
	server.Post("/bar/ticket/status",update_ticket_status);
	server.Get("/bar",get_tickets_json);
}

void BarController::bar_screen(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "bar") {
		res.set_redirect("/login/bar");
		return;
	}

	auto tickets = TicketService::get_instance().get_pending_tickets();
	res.set_content(HtmlTemplates::bar_screen(session,tickets),"text/html");
}

void BarController::update_ticket_status(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "bar") {
		res.set_redirect("/login/bar");
		return;
	}

	std::string ticket_id = req.get_param_value("ticket_id");
	std::string estado = req.get_param_value("estado");

	LOG_INFO("BarController::update_ticket_status - Ticket status update: ticket='" + ticket_id + "', status='" + estado + "'");
	TicketService::get_instance().update_ticket_status(ticket_id,estado);

	res.set_redirect("/bar");
}

void BarController::get_tickets_json(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().get_session_from_request(req);
	if(!session || session->role != "bar") {
		res.status = 401;
		res.set_content("{\"error\":\"unauthorized\"}","application/json");
		return;
	}

	auto tickets = TicketService::get_instance().get_pending_tickets();

	std::ostringstream json;
	json << "[";
	for(size_t i = 0;i<tickets.size();i++) {
		const auto& t = tickets[i];
		json << "{";
		json << "\"id\":\"" << t.id << "\",";
		json << "\"vendedor\":\"" << t.vendor_name << "\",";
		json << "\"fecha\":\"" << t.fecha_creacion << "\",";
		json << "\"estado\":\"" << t.estado << "\",";
		json << "\"items\":[";
		for(size_t j = 0; j<t.items.size();j++) {
			const auto& item = t.items[j];
			json << "{\"nombre\":\"" << item.name << "\",\"cantidad\":" << item.quantity << "}";
			if(j<t.items.size()-1) json << ",";
		}
		json << "]}";
		if(i<tickets.size()-1) json << ",";
	}
	json << "]";

	res.set_content(json.str(),"application/json");
}

