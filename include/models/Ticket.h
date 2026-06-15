#ifndef MODELS_TICKET_H
#define	MODELS_TICKET_H	1

#include <string>
#include <vector>
#include <models/CartItem.h>

struct TicketItem {
	std::string product_id;
	std::string name;
	int quantity;
};

struct Ticket {
	std::string id;
	std::string visitor_id;
	std::string vendor_name;
	std::string fecha_creacion;
	std::string estado;	// "PENDIENTE, "EN_PROCESO", "COMPLETADO"
	std::vector<TicketItem> items;
};

#endif

