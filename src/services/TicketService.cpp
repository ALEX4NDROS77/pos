#include <services/TicketService.h>
#include <services/DatabaseService.h>
#include <utils/Logger.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>
#include <sqlite3.h>

TicketService& TicketService::get_instance() {
	static TicketService instance;
	return instance;
}

std::optional<std::string> TicketService::create_ticket(Session* session,const std::string& orden_id) {
	if(!session || session->cart.empty()) {
		return std::nullopt;
	}

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string ticket_id = "TKT-" + Utils::generate_uuid().substr(0,8);
	std::string fecha = Utils::get_timestamp();

	{
		SqliteStatement ticket_stmt(db.get_connection(),"INSERT INTO TICKETS (ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO) VALUES (?,?,?,?,'PENDIENTE');");
		if(!ticket_stmt.ok()) return std::nullopt;

		ticket_stmt.bind(1,ticket_id);
		ticket_stmt.bind(2,orden_id);
		ticket_stmt.bind(3,session->username);
		ticket_stmt.bind(4,fecha);

		if(!ticket_stmt.exec()) return std::nullopt;
	}

	SqliteStatement item_stmt(db.get_connection(),"INSERT INTO TICKET_ITEMS (TICKET_ID,PRODUCTO_ID,NOMBRE_PRODUCTO,CANTIDAD) VALUES (?,?,?,?);");
	if(!item_stmt.ok()) return std::nullopt;

	sqlite3_exec(db.get_connection(),"BEGIN TRANSACTION;",nullptr,nullptr,nullptr);

	for(const auto& item : session->cart) {
		item_stmt.reset();

		item_stmt.bind(1,ticket_id);
		item_stmt.bind(2,item.product_id);
		item_stmt.bind(3,item.name);
		item_stmt.bind(4,item.quantity);

		item_stmt.exec();
	}

	sqlite3_exec(db.get_connection(),"COMMIT;",nullptr,nullptr,nullptr);

	return ticket_id;
}

std::vector<Ticket> TicketService::get_pending_tickets() {
	std::vector<Ticket> tickets;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),R"(
		SELECT T.ID_TICKET,T.ORDEN_ID,T.VENDEDOR_NOMBRE,T.FECHA_CREACION,T.ESTADO,O.METODO_PAGO
		FROM TICKETS T
		JOIN ORDENES O ON T.ORDEN_ID = O.ID_VENTA
		WHERE T.ESTADO IN ('PENDIENTE','EN_PROCESO')
		ORDER BY T.FECHA_CREACION ASC;
	)");
	if(!stmt.ok()) return tickets;

	while(stmt.step() == SQLITE_ROW) {
		Ticket t;
		t.id = stmt.column_text(0);
		t.visitor_id = stmt.column_text(1);
		t.vendor_name = stmt.column_text(2);
		t.fecha_creacion = stmt.column_text(3);
		t.estado = stmt.column_text(4);
		std::string metodo = stmt.column_text(5);
		t.metodo_pago = metodo.empty() ? 'E' : metodo[0];
		tickets.push_back(t);
	}

	for(auto& ticket : tickets) {
		ticket.items = get_ticket_items_locked(ticket.id,db);
	}

	return tickets;
}

std::vector<Ticket> TicketService::get_all_tickets() {
	std::vector<Ticket> tickets;
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO FROM TICKETS ORDER BY FECHA_CREACION DESC;");
	if(!stmt.ok()) return tickets;

	while(stmt.step() == SQLITE_ROW) {
		Ticket t;
		t.id = stmt.column_text(0);
		t.visitor_id = stmt.column_text(1);
		t.vendor_name = stmt.column_text(2);
		t.fecha_creacion = stmt.column_text(3);
		t.estado = stmt.column_text(4);
		tickets.push_back(t);
	}

	return tickets;
}

std::optional<Ticket> TicketService::get_ticket_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO FROM TICKETS WHERE ID_TICKET = ?;");
	if(!stmt.ok()) return std::nullopt;

	stmt.bind(1,id);

	if(stmt.step() != SQLITE_ROW) return std::nullopt;

	Ticket t;
	t.id = stmt.column_text(0);
	t.visitor_id = stmt.column_text(1);
	t.vendor_name = stmt.column_text(2);
	t.fecha_creacion = stmt.column_text(3);
	t.estado = stmt.column_text(4);

	t.items = get_ticket_items_locked(t.id,db);
	return t;
}

bool TicketService::update_ticket_status(const std::string& id,const std::string& estado) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"UPDATE TICKETS SET ESTADO = ? WHERE ID_TICKET = ?;");
	if(!stmt.ok()) {
		LOG_ERROR("TicketService::update_ticket_status - failed to prepare SQL");
		return false;
	}

	stmt.bind(1,estado);
	stmt.bind(2,id);

	bool success = stmt.exec();

	if(success) {
		LOG_INFO("TicketSErvice::update_ticket_status - ticket='" + id + "' -> '" + estado + "'");
	} else {
		LOG_ERROR("TicketSErvice::update_ticket_status - failed to update ticket='" + id + "'");
	}

	return success;
}

std::vector<TicketItem> TicketService::get_ticket_items(const std::string& ticket_id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());
	return get_ticket_items_locked(ticket_id,db);
}

std::vector<TicketItem> TicketService::get_ticket_items_locked(const std::string& ticket_id,DatabaseService& db) {
	std::vector<TicketItem> items;

	SqliteStatement stmt(db.get_connection(),"SELECT PRODUCTO_ID,NOMBRE_PRODUCTO,CANTIDAD FROM TICKET_ITEMS WHERE TICKET_ID = ?;");
	if(!stmt.ok()) return items;

	stmt.bind(1,ticket_id);

	while(stmt.step() == SQLITE_ROW) {
		TicketItem item;
		item.product_id = stmt.column_text(0);
		item.name = stmt.column_text(1);
		item.quantity = stmt.column_int(2);
		items.push_back(item);
	}

	return items;
}
