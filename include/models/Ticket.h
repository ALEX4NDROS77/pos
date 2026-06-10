#ifndef MODELS_TICKET_H
#define	MODELS_TICKET_H	1

#include <string>
#include <vector>
#include <CartItem.h>

struct TicketItem {
	std::string productoId;
	std::string nombre;
	int cantidad;
}:

struct Ticket {
	std::string id;
	std::string visitorId;
	std::string vendorNombre;
	std::string fechaCreacion;
	std::string estado;	// "PENDIENTE, "EN_PROCESO", "COMPLETADO"
	std::vector<TicketItem> items;
};

#endif

