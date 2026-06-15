#include <sqlite3.h>
#include <services/CartService.h>
#include <services/TicketService.h>
#include <services/DatabaseService.h>
#include <services/InventoryService.h>
#include <services/SalesService.h>
#include <utils/Utils.h>
#include <views/HtmlTemplates.h>

SalesService& SalesService::get_instance() {
	static SalesService instance;
	return instance;
}

std::optional<std::string> SalesService::checkout(Session* session,char payment_method) {
	if(!session || session->cart.empty()) {
		return std::nullopt;
	}

	// Re-validates availabitily of the producto to avoid run-conditions
	for(const auto& item : session->cart) {
		auto current_product = InventoryService::get_instance().get_product_by_id(item.product_id);
		if(current_product->quantity < item.quantity) {
			return "Error: El producto '" + item.name + "' ya no tiene stock suficiente.";
		}
	}

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string order_id = "ORD-" + Utils::generate_uuid().substr(0,8);
	std::string date = Utils::get_current_time();
	double total = CartService::get_instance().get_cart_total(session);

	const char* order_sql = "INSERT INTO ORDENES (ID_VENTA, FECHA_VENTA, VENTA_TOTAL, METODO_PAGO, VENDEDOR) VALUES (?, ?, ?, ?, ?);";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),order_sql,-1,&stmt,nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	std::string username = session->username;
	char method[2] = {payment_method,'\0'};

	sqlite3_bind_text(stmt,1,order_id.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,2,date.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_double(stmt,3,total);
	sqlite3_bind_text(stmt,4,method,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,5,username.c_str(),-1,SQLITE_STATIC);

	if(sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}
	sqlite3_finalize(stmt);

	const char* sale_sql = "INSERT INTO VENTAS (ID_VENTA, ORDEN_ID, PRODUCTO_ID, CANTIDAD_VENTA, PRECIO_UNITARIO) VALUES (?, ?, ?, ?, ?);";
	const char* update_stock_sql = "UPDATE PRODUCTOS SET CANTIDAD = CANTIDAD - ? WHERE ID_PRODUCTO = ?;";

	for(const auto& item : session->cart) {
		std::string sale_id = "VTA-" + Utils::generate_uuid().substr(0,8);

		if(sqlite3_prepare_v2(db.get_connection(),sale_sql,-1,&stmt,nullptr) != SQLITE_OK) {
			continue;
		}

		sqlite3_bind_text(stmt,1,sale_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,2,order_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,3,item.product_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_int(stmt,4,item.quantity);
		sqlite3_bind_double(stmt,5,item.unit_price);

		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		if(sqlite3_prepare_v2(db.get_connection(),update_stock_sql,-1,&stmt,nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt,1,item.quantity);
			sqlite3_bind_text(stmt,2,item.product_id.c_str(),-1,SQLITE_STATIC);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}
	}

	std::string ticket_id = "TKT-" + Utils::generate_uuid().substr(0,8);
	std::string ticket_fecha = Utils::get_timestamp();

	const char* ticket_sql = "INSERT INTO TICKETS (ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO) VALUES (?,?,?,?,'PENDIENTE');";
	if(sqlite3_prepare_v2(db.get_connection(),ticket_sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,ticket_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,2,order_id.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,3,session->username.c_str(),-1,SQLITE_STATIC);
		sqlite3_bind_text(stmt,4,ticket_fecha.c_str(),-1,SQLITE_STATIC);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		const char* item_sql = "INSERT INTO TICKET_ITEMS (TICKET_ID,PRODUCTO_ID,NOMBRE_PRODUCTO,CANTIDAD) VALUES (?,?,?,?);";
		for(const auto& item : session->cart) {
			if(sqlite3_prepare_v2(db.get_connection(),item_sql,-1,&stmt,nullptr) != SQLITE_OK) {
				continue;
			}

			sqlite3_bind_text(stmt,1,ticket_id.c_str(),-1,SQLITE_STATIC);
			sqlite3_bind_text(stmt,2,item.product_id.c_str(),-1,SQLITE_STATIC);
			sqlite3_bind_text(stmt,3,item.name.c_str(),-1,SQLITE_STATIC);
			sqlite3_bind_int(stmt,4,item.quantity);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}
	}

	session->cart.clear();

	return order_id;
}

SalesReport SalesService::get_sales_report() {
	SalesReport report;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_VENTA, FECHA_VENTA, VENTA_TOTAL, METODO_PAGO, VENDEDOR FROM ORDENES ORDER BY FECHA_VENTA DESC;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		while(sqlite3_step(stmt) == SQLITE_ROW) {
			Order order;
			order.sale_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
			order.sale_time = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
			order.total_sale = sqlite3_column_double(stmt,2);
			const char* method = reinterpret_cast<const char*>(sqlite3_column_text(stmt,3));
			order.vendor = reinterpret_cast<const char*>(sqlite3_column_text(stmt,4));
			order.payment_method = method ? method[0] : 'C';

			report.transactions[order.payment_method].revenue += order.total_sale;
			report.transactions[order.payment_method].transaction_count++;
			report.orders.push_back(order);
		}
		sqlite3_finalize(stmt);
	}

	report.total_revenue = 0.0;
	report.total_transactions = 0;
	
	for(const auto& it : report.transactions) {
		report.total_revenue += it.second.revenue;
		report.total_transactions += it.second.transaction_count;
	}

	return report;
}

std::vector<Sale> SalesService::get_sales_by_order_id(const std::string& order_id) {
	std::vector<Sale> sales;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	const char* sql = "SELECT ID_VENTA, ORDEN_ID, PRODUCTO_ID, CANTIDAD_VENTA, PRECIO_UNITARIO FROM VENTAS WHERE ORDEN_ID = ?;";
	sqlite3_stmt* stmt;

	if(sqlite3_prepare_v2(db.get_connection(),sql,-1,&stmt,nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt,1,order_id.c_str(),-1,SQLITE_STATIC);

		while(sqlite3_step(stmt) == SQLITE_ROW) {
			Sale sale;
			sale.sale_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
			sale.order_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
			sale.product_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
			sale.sale_quantity = sqlite3_column_int(stmt,3);
			sale.unit_price = sqlite3_column_double(stmt,4);
			sales.push_back(sale);
		}
		sqlite3_finalize(stmt);
	}

	return sales;
}

