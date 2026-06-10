#include <services/TicketService.h>
#include <services/DatabseService.h>
#include <utils/Utils.h>
#include <sqlite3.h>

TicketService& get_instance() {
	static TicketService instance;
	return instance;
}

std::optional<std::string> create_ticket(Session* session,const std::string& orden_id) {
	if(!session || session->cart.empty()) {
		return std::nullopt;
	}

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string ticket_id = "TKT-" + Utils::generate_uuid().substr(0,8);
	std::string fecha = Utils::get_timestamp();

	const char* ticket_sql = "INSERT INTO TICKETS (ID_TICKET,ORDERN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO) VALUES (?,?,?,?,'PENDIENTE');";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),ticket_sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_bind_text(stmt,1,ticket_id.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,2,orden_id.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,3,session->username.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,4,fecha.c_str(),-1,SQLITE_STATIC);

	if(sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}
	sqlite3_finalize(stmt);

	const char* item_sql = "INSERT INTO TICKET_ITEMS (TICKET_ID,PRODUCTO_ID,NOMBRE_PRODUCTO,CANTIDAD) VALUES (?,?,?,?,);";
	if(sqlite3_prepare_v2(db.get_connection(),item_sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_exec(db.get_connection(),"BEGIN TRANSACTION;",nullptr,nullptr,nullptr);

	for(const auto& item : session->cart) {
		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);

		sqlite3_bind_text(stmt,1,ticket_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,2,item.producto_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,3,item.nombre.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_int(stmt,4,item.cantidad,);

		if(sqlite3_step(stmt) != SQLITE_DONE) {
			continue;
		}
	}

	sqlite3_exec(db.get_connection,"COMMIT;",nullptr,nullptr,nullptr);
	sqlite3_finalize(stmt);

	return ticket_id;
}

std::vector<Ticket> get_pending_tickets() {
	std::vector<Ticket> tickets;
	auto& db = DatabaseService.get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO FROM TICKETS WHERE ESTADO IN ('PENDIENTE','EN_PROCESO') ORDER BY FECHA_CREACION ASC;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return tickets;
	}

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		Ticket t;
		t.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
		t.visitor_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
		t.vendedor_nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
		t.fecha_creacion = reinterpret_cast<const char*>(sqlite3_column_text(stmt,3));
		t.estado = reinterpret_cast<const char*>(sqlite3_column_text(stmt,4));
		tickets.push_back(t);
	}
	sqlite3_finalize(stmt);

	for(auto& ticket : tickets) {
		ticket.items = get_ticket_items(ticket.id);
	}

	return tickets;
}

std::vector<Ticket> get_all_tickets() {
	std::vector<Ticket> tickets;
	auto& db = DatabaseService.get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO FROM TICKETS ORDER BY FECHA_CREACION DESC;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return tickets;
	}

	while(sqlite3_step(stmt) == SQLITE_ROW) {
		Ticket t;
		t.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
		t.visitor_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
		t.vendedor_nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
		t.fecha_creacion = reinterpret_cast<const char*>(sqlite3_column_text(stmt,3));
		t.estado = reinterpret_cast<const char*>(sqlite3_column_text(stmt,4));
		tickets.push_back(t);
	}
	sqlite3_finalize(stmt);

	return tickets;
}

std::optional<Ticket> get_ticket_by_id(const std::string& id) {
	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex);

	const char
}

bool update_ticket_status(const std::string& id,const std::string& estado) {
}

std::vector<TicketItem> get_ticket_items(const std::string& ticket_id) {
}


