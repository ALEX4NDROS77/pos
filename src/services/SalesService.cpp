#include <mutex>
#include <sqlite3.h>
#include <services/CartService.h>
#include <services/DatabaseService.h>
#include <services/InventoryService.h>
#include <services/SalesService.h>
#include <services/TicketService.h>
#include <utils/Logger.h>
#include <utils/SqliteStatement.h>
#include <utils/Utils.h>
#include <vector>
#include <views/HtmlTemplates.h>

SalesService& SalesService::get_instance() {
	static SalesService instance;
	return instance;
}

std::optional<std::string> SalesService::checkout(Session* session,char payment_method) {
	if(!session || session->cart.empty()) {
		LOG_WARNING("SalesService::checkout - empty session or cart");
		return std::nullopt;
	}

	bool sell_from_isle = session->role == "vendor";
	if(sell_from_isle && session->isla_id.empty()) {
		LOG_WARNING("SalesService::checkout - vendor with no isle assigned: " + session->username);
		return "Error: Tu usuario no tiene una isla asignada. Contacta al administrador.";
	}

	LOG_DEBUG("SalesService::checkout - validating stock for " + std::to_string(session->cart.size()) + " items");

	// Re-validates availabitily of the producto to avoid run-conditions
	for(const auto& item : session->cart) {
		int available = 0;
		if(sell_from_isle) {
			available = InventoryService::get_instance().get_isla_stock(session->isla_id,item.product_id);
		} else {
			auto current_product = InventoryService::get_instance().get_product_by_id(item.product_id);
			available = current_product ? current_product->quantity : 0;
		}
		if(available < item.quantity) {
			LOG_WARNING("SalesService::checkout - insuficient stock for '" + item.name + "'");
			return "Error: El producto '" + item.name + "' ya no tiene stock suficiente.";
		}
	}

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string order_id = "ORD-" + Utils::generate_uuid().substr(0,8);
	std::string date = Utils::get_current_time();
	double total = CartService::get_instance().get_cart_total(session);
	std::string username = session->username;
	char method[2] = {payment_method,'\0'};

	sqlite3_exec(db.get_connection(),"BEGIN IMMEDIATE;",nullptr,nullptr,nullptr);

	{
		SqliteStatement order_stmt(db.get_connection(),"INSERT INTO ORDENES (ID_VENTA, FECHA_VENTA, VENTA_TOTAL, METODO_PAGO, VENDEDOR, ISLA_ID) VALUES (?, ?, ?, ?, ?, ?);");
		if(!order_stmt.ok()) {
			LOG_ERROR("SalesService::checkout - failed to prepare order SQL");
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}

		order_stmt.bind(1,order_id);
		order_stmt.bind(2,date);
		order_stmt.bind(3,total);
		order_stmt.bind(4,method);
		order_stmt.bind(5,username);
		order_stmt.bind(6,session->isla_id);

		if(!order_stmt.exec()) {
			LOG_ERROR("SalesService::checkout - failed to insert order");
			sqlite3_exec(db.get_connection(),"ROLLBACK;",nullptr,nullptr,nullptr);
			return std::nullopt;
		}
	}

	LOG_DEBUG("SalesService::checkout - order created: " + order_id);

	{
		SqliteStatement sale_stmt(db.get_connection(),"INSERT INTO VENTAS (ID_VENTA, ORDEN_ID, PRODUCTO_ID, CANTIDAD_VENTA, PRECIO_UNITARIO) VALUES (?, ?, ?, ?, ?);");
		SqliteStatement update_stock_stmt(db.get_connection(),sell_from_isle
			? "UPDATE INVENTARIO_ISLA SET CANTIDAD = CANTIDAD - ? WHERE ISLA_ID = ? AND PRODUCTO_ID = ?;"
			: "UPDATE PRODUCTOS SET CANTIDAD = CANTIDAD - ? WHERE ID_PRODUCTO = ?;");

		for(const auto& item : session->cart) {
			std::string sale_id = "VTA-" + Utils::generate_uuid().substr(0,8);

			if(sale_stmt.ok()) {
				sale_stmt.reset();
				sale_stmt.bind(1,sale_id);
				sale_stmt.bind(2,order_id);
				sale_stmt.bind(3,item.product_id);
				sale_stmt.bind(4,item.quantity);
				sale_stmt.bind(5,item.unit_price);
				sale_stmt.exec();
			}

			if(update_stock_stmt.ok()) {
				update_stock_stmt.reset();
				if(sell_from_isle) {
					update_stock_stmt.bind(1,item.quantity);
					update_stock_stmt.bind(2,session->isla_id);
					update_stock_stmt.bind(3,item.product_id);
				} else {
					update_stock_stmt.bind(1,item.quantity);
					update_stock_stmt.bind(2,item.product_id);
				}
				update_stock_stmt.exec();
			}
		}
	}

	std::string ticket_id = "TKT-" + Utils::generate_uuid().substr(0,8);
	std::string ticket_fecha = Utils::get_timestamp();

	{
		SqliteStatement ticket_stmt(db.get_connection(),"INSERT INTO TICKETS (ID_TICKET,ORDEN_ID,VENDEDOR_NOMBRE,FECHA_CREACION,ESTADO) VALUES (?,?,?,?,'PENDIENTE');");
		if(ticket_stmt.ok()) {
			ticket_stmt.bind(1,ticket_id);
			ticket_stmt.bind(2,order_id);
			ticket_stmt.bind(3,session->username);
			ticket_stmt.bind(4,ticket_fecha);
			ticket_stmt.exec();

			SqliteStatement item_stmt(db.get_connection(),"INSERT INTO TICKET_ITEMS (TICKET_ID,PRODUCTO_ID,NOMBRE_PRODUCTO,CANTIDAD) VALUES (?,?,?,?);");
			if(item_stmt.ok()) {
				for(const auto& item : session->cart) {
					item_stmt.reset();
					item_stmt.bind(1,ticket_id);
					item_stmt.bind(2,item.product_id);
					item_stmt.bind(3,item.name);
					item_stmt.bind(4,item.quantity);
					item_stmt.exec();
				}
			}
		}
	}

	sqlite3_exec(db.get_connection(),"COMMIT;",nullptr,nullptr,nullptr);

	session->cart.clear();

	LOG_INFO("SalesService::checkout - completed order=" + order_id + ", ticket=" + ticket_id + ", total=$" + std::to_string(total));
	return order_id;
}

SalesReport SalesService::get_sales_report(const std::string& vendor,const std::string& product_id) {
	SalesReport report;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	std::string sql = "SELECT DISTINCT O.ID_VENTA, O.FECHA_VENTA, O.VENTA_TOTAL, O.METODO_PAGO, O.VENDEDOR FROM ORDENES O";
	if(!product_id.empty()) {
		sql += " JOIN VENTAS V ON V.ORDEN_ID = O.ID_VENTA";
	}

	std::vector<std::string> conditions;
	if(!vendor.empty()) conditions.push_back("O.VENDEDOR = ?");
	if(!product_id.empty()) conditions.push_back("V.PRODUCTO_ID = ?");

	if(!conditions.empty()) {
		sql += " WHERE ";
		for(size_t i = 0;i < conditions.size();++i) {
			if(i > 0) sql += " AND ";
			sql += conditions[i];
		}
	}
	sql += " ORDER BY O.FECHA_VENTA DESC;";

	SqliteStatement stmt(db.get_connection(),sql);
	if(stmt.ok()) {
		int idx = 1;
		if(!vendor.empty()) stmt.bind(idx++,vendor);
		if(!product_id.empty()) stmt.bind(idx++,product_id);

		while(stmt.step() == SQLITE_ROW) {
			Order order;
			order.sale_id = stmt.column_text(0);
			order.sale_time = stmt.column_text(1);
			order.total_sale = stmt.column_double(2);
			std::string method = stmt.column_text(3);
			order.vendor = stmt.column_text(4);
			order.payment_method = method.empty() ? 'C' : method[0];

			report.transactions[order.payment_method].revenue += order.total_sale;
			report.transactions[order.payment_method].transaction_count++;
			report.orders.push_back(order);
		}
	}

	report.total_revenue = 0.0;
	report.total_transactions = 0;

	for(const auto& it : report.transactions) {
		report.total_revenue += it.second.revenue;
		report.total_transactions += it.second.transaction_count;
	}

	return report;
}

SalesReport SalesService::get_isle_sales_summary(const std::string& isla_id) {
	SalesReport report;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT METODO_PAGO, VENTA_TOTAL FROM ORDENES WHERE ISLA_ID = ?;");
	if(stmt.ok()) {
		stmt.bind(1,isla_id);

		while(stmt.step() == SQLITE_ROW) {
			std::string method = stmt.column_text(0);
			double total = stmt.column_double(1);
			char key = method.empty() ? 'C' : method[0];

			report.transactions[key].revenue += total;
			report.transactions[key].transaction_count++;
		}
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

	SqliteStatement stmt(db.get_connection(),"SELECT ID_VENTA, ORDEN_ID, PRODUCTO_ID, CANTIDAD_VENTA, PRECIO_UNITARIO FROM VENTAS WHERE ORDEN_ID = ?;");
	if(stmt.ok()) {
		stmt.bind(1,order_id);

		while(stmt.step() == SQLITE_ROW) {
			Sale sale;
			sale.sale_id = stmt.column_text(0);
			sale.order_id = stmt.column_text(1);
			sale.product_id = stmt.column_text(2);
			sale.sale_quantity = stmt.column_int(3);
			sale.unit_price = stmt.column_double(4);
			sales.push_back(sale);
		}
	}

	return sales;
}

std::vector<std::string> SalesService::get_all_vendors_with_sales() {
	std::vector<std::string> vendors;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT DISTINCT VENDEDOR FROM ORDENES ORDER BY VENDEDOR;");
	if(stmt.ok()) {
		while(stmt.step() == SQLITE_ROW) {
			std::string vendor = stmt.column_text(0);
			if(!vendor.empty()) {
				vendors.push_back(vendor);
			}
		}
	}
	return vendors;
}

std::vector<Product> SalesService::get_all_products_with_sales() {
	std::vector<Product> products;

	auto& db = DatabaseService::get_instance();
	std::lock_guard<std::mutex> lock(db.get_mutex());

	SqliteStatement stmt(db.get_connection(),"SELECT DISTINCT P.ID_PRODUCTO,P.NOMBRE FROM VENTAS V JOIN PRODUCTOS P ON P.ID_PRODUCTO = V.PRODUCTO_ID ORDER BY P.NOMBRE;");
	if(stmt.ok()) {
		while(stmt.step() == SQLITE_ROW) {
			Product product;
			product.id = stmt.column_text(0);
			product.name = stmt.column_text(1);
			products.push_back(product);
		}
	}
	return products;
}
