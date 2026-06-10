#ifndef SERVICES_TICKETSERVICE_H
#define	SERVICES_TICKETSERVICE_H	1

#include <models/Ticket.h>
#include <models/Session.h>
#include <vector>
#include <optional>

class TicketService {
	public:
		static TicketService& get_instance();

		std::optional<std::string> create_ticket(Session* session,const std::string& orden_id);
		std::vector<Ticket> get_pending_tickets();
		std::vector<Ticket> get_all_tickets();
		std::optional<Ticket> get_ticket_by_id(const std::string& id);
		bool update_ticket_status(const std::string& id,const std::string& estado);
		std::vector<TicketItem> get_ticket_items(const std::string& ticket_id);

	private:
		TicketService() = default;
		TicketService(const TicketService&) = delete;
		TicketService& operator=(const TicketService&) = delete;
};

#endif

