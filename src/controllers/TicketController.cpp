#include <controllers/TicketController.h>
#include <services/SessionService.h>
#include <services/TicketService.h>
#include <utils/Logger.h>

void TicketController::register_routes(httplib::Server& server) {
	server.Post("/ticket/complete",complete_ticket);
}

void TicketController::complete_ticket(const httplib::Request& req,httplib::Response& res) {
	auto* session = SessionService::get_instance().require_session(req,res);
	if(!session) return;

	std::string ticket_id = req.get_param_value("ticket_id");

	LOG_INFO("TicketController::complete_ticket - Ticket completed: ticket='" + ticket_id + "', by='" + session->username + "'");
	TicketService::get_instance().update_ticket_status(ticket_id,"COMPLETADO");

	res.set_redirect("/home");
}
